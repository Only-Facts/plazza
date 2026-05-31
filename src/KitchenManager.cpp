#include "KitchenManager.hpp"
#include "Kitchen.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "PipeIPC.hpp"

#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

KitchenManager::KitchenManager(int cooksPerKitchen, double multiplier, int restockTime)
  : _cooksPerKitchen(cooksPerKitchen),
    _multiplier(multiplier),
    _restockTime(restockTime),
    _nextKitchenId(1),
    _shuttingDown(false)
{
}

KitchenManager::~KitchenManager()
{
  shutdown();
}

static void runKitchenChild(
  int kitchenId,
  int cooksPerKitchen,
  double multiplier,
  int restockTime,
  PipeIPC fromReception,
  PipeIPC toReception
)
{
  auto lastActivity = std::chrono::steady_clock::now();

  Kitchen kitchen(kitchenId, cooksPerKitchen, multiplier, restockTime, [&toReception](const Pizza& pizza) {
    try {
      Message doneMessage(MessageType::PizzaDone, pizza);
      toReception.send(doneMessage.pack());
    } catch (...) {
    }
  });

  struct pollfd pfd;
  pfd.fd = fromReception.getReadFd();
  pfd.events = POLLIN;
  pfd.revents = 0;

  while (true) {
    int ret = poll(&pfd, 1, 100);

    if (ret < 0) {
      if (errno == EINTR)
        continue;
      break;
    }

    if (ret > 0) {
      if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))
        break;

      if (pfd.revents & POLLIN) {
        try {
          Message message = Message::unpack(fromReception.receive());

          if (message.getType() == MessageType::NewPizza && message.hasPizza()) {
            kitchen.addPizza(message.getPizza());
            lastActivity = std::chrono::steady_clock::now();
          } else if (message.getType() == MessageType::StatusRequest) {
            Message response(MessageType::StatusResponse, kitchen.getStatusString());
            toReception.send(response.pack());
          } else if (message.getType() == MessageType::KitchenClosing) {
            break;
          }
        } catch (...) {
          break;
        }
      }
    }

    if (kitchen.getLoad() == 0) {
      auto now = std::chrono::steady_clock::now();
      auto idleMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActivity).count();
      if (idleMs >= 5000) {
        try {
          Message closeMessage(MessageType::KitchenClosing);
          toReception.send(closeMessage.pack());
        } catch (...) {
        }
        break;
      }
    } else {
      lastActivity = std::chrono::steady_clock::now();
    }
  }

  std::exit(0);
}

KitchenProcess& KitchenManager::createKitchenProcess()
{
  int pipeToChild[2];
  int pipeToParent[2];

  if (pipe(pipeToChild) == -1)
    throw std::runtime_error("pipe to child failed");
  if (pipe(pipeToParent) == -1) {
    close(pipeToChild[0]);
    close(pipeToChild[1]);
    throw std::runtime_error("pipe to parent failed");
  }

  int id = _nextKitchenId++;
  int capacity = _cooksPerKitchen * 2;
  pid_t pid = fork();

  if (pid == -1) {
    close(pipeToChild[0]);
    close(pipeToChild[1]);
    close(pipeToParent[0]);
    close(pipeToParent[1]);
    throw std::runtime_error("fork failed");
  }

  if (pid == 0) {
    PipeIPC fromReception(pipeToChild[0], -1);
    PipeIPC toReception(-1, pipeToParent[1]);

    close(pipeToChild[1]);
    close(pipeToParent[0]);

    runKitchenChild(id, _cooksPerKitchen, _multiplier, _restockTime, std::move(fromReception), std::move(toReception));
  }

  PipeIPC toKitchen(-1, pipeToChild[1]);
  PipeIPC fromKitchen(pipeToParent[0], -1);

  close(pipeToChild[0]);
  close(pipeToParent[1]);

  _processKitchens.push_back(std::make_unique<KitchenProcess>(id, pid, std::move(toKitchen), std::move(fromKitchen), capacity));

  std::cout << "Kitchen process " << id << " created with pid " << pid << std::endl;
  Logger::log("Kitchen " + std::to_string(id) + " created with pid " + std::to_string(pid));
  return *_processKitchens.back();
}

KitchenProcess& KitchenManager::getBestKitchenProcess()
{
  KitchenProcess* bestKitchen = nullptr;

  for (auto& kitchen : _processKitchens) {
    if (!kitchen->canAcceptPizza())
      continue;
    if (bestKitchen == nullptr || kitchen->getLoad() < bestKitchen->getLoad())
      bestKitchen = kitchen.get();
  }

  if (bestKitchen == nullptr)
    return createKitchenProcess();
  return *bestKitchen;
}

void KitchenManager::assignPizzaToProcess(const Pizza& pizza)
{
  KitchenProcess& kitchen = getBestKitchenProcess();

  std::cout << "[Reception] Sending " << pizza.typeToString() << " " << pizza.sizeToString()
            << " to kitchen " << kitchen.getId() << std::endl;

  try {
    kitchen.sendPizza(pizza);
    kitchen.incrementLoad();
    Logger::log("Sent " + pizza.typeToString() + " " + pizza.sizeToString() + " to kitchen " + std::to_string(kitchen.getId()));
  } catch (const std::exception& error) {
    kitchen.markClosing();
    Logger::log("Failed to send pizza to kitchen " + std::to_string(kitchen.getId()) + ": " + error.what());
    throw;
  }
}

bool KitchenManager::handleKitchenMessage(std::size_t index)
{
  if (index >= _processKitchens.size())
    return false;

  Message message;
  KitchenProcess& kitchen = *_processKitchens[index];

  if (!kitchen.receiveMessage(message)) {
    kitchen.markClosing();
    return false;
  }

  if (message.getType() == MessageType::PizzaDone && message.hasPizza()) {
    Pizza pizza = message.getPizza();
    kitchen.decrementLoad();
    std::cout << "\n[Reception] Pizza ready: " << pizza.typeToString() << " " << pizza.sizeToString()
              << " (Kitchen " << kitchen.getId() << ")" << std::endl;
    Logger::log("Pizza ready: " + pizza.typeToString() + " " + pizza.sizeToString() + " from kitchen " + std::to_string(kitchen.getId()));
    return false;
  }

  if (message.getType() == MessageType::StatusResponse && message.hasText()) {
    std::cout << message.getText();
    return true;
  }

  if (message.getType() == MessageType::KitchenClosing) {
    std::cout << "\n[Reception] Kitchen " << kitchen.getId() << " closed after inactivity." << std::endl;
    Logger::log("Kitchen " + std::to_string(kitchen.getId()) + " closed after inactivity");
    kitchen.markClosing();
    return false;
  }

  return false;
}

void KitchenManager::update()
{
  if (_processKitchens.empty())
    return;

  std::vector<struct pollfd> pfds;
  pfds.reserve(_processKitchens.size());

  for (const auto& kitchen : _processKitchens) {
    struct pollfd pfd;
    pfd.fd = kitchen->getReadFd();
    pfd.events = POLLIN | POLLHUP | POLLERR;
    pfd.revents = 0;
    pfds.push_back(pfd);
  }

  int ret = poll(pfds.data(), pfds.size(), 0);
  if (ret > 0) {
    for (std::size_t i = 0; i < pfds.size() && i < _processKitchens.size(); ++i) {
      if (pfds[i].revents & POLLIN)
        handleKitchenMessage(i);
      else if (pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
        _processKitchens[i]->markClosing();
    }
  }

  cleanupDeadProcesses();
  removeClosedProcesses();
}

void KitchenManager::displayStatus()
{
  std::cout << "=== PLAZZA STATUS ===" << std::endl;

  if (_processKitchens.empty()) {
    std::cout << "No active kitchens." << std::endl;
    return;
  }

  for (const auto& kitchen : _processKitchens) {
    try {
      kitchen->sendMessage(Message(MessageType::StatusRequest));
    } catch (...) {
      kitchen->markClosing();
    }
  }

  std::size_t expectedResponses = 0;
  for (const auto& kitchen : _processKitchens) {
    if (!kitchen->isClosing())
      expectedResponses++;
  }

  std::size_t receivedResponses = 0;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(800);

  while (receivedResponses < expectedResponses && std::chrono::steady_clock::now() < deadline) {
    std::vector<struct pollfd> pfds;
    pfds.reserve(_processKitchens.size());

    for (const auto& kitchen : _processKitchens) {
      struct pollfd pfd;
      pfd.fd = kitchen->getReadFd();
      pfd.events = POLLIN | POLLHUP | POLLERR;
      pfd.revents = 0;
      pfds.push_back(pfd);
    }

    int ret = poll(pfds.data(), pfds.size(), 50);
    if (ret <= 0)
      continue;

    for (std::size_t i = 0; i < pfds.size() && i < _processKitchens.size(); ++i) {
      if (pfds[i].revents & POLLIN) {
        if (handleKitchenMessage(i))
          receivedResponses++;
      } else if (pfds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
        _processKitchens[i]->markClosing();
      }
    }
  }

  cleanupDeadProcesses();
  removeClosedProcesses();
}

void KitchenManager::cleanupDeadProcesses()
{
  for (auto& kitchen : _processKitchens) {
    int status = 0;
    pid_t result = waitpid(kitchen->getPid(), &status, WNOHANG);
    if (result > 0)
      kitchen->markClosing();
  }
}

void KitchenManager::removeClosedProcesses()
{
  for (auto it = _processKitchens.begin(); it != _processKitchens.end();) {
    if ((*it)->isClosing()) {
      int status = 0;
      waitpid((*it)->getPid(), &status, WNOHANG);
      it = _processKitchens.erase(it);
    } else {
      ++it;
    }
  }
}

void KitchenManager::shutdown()
{
  if (_shuttingDown)
    return;

  _shuttingDown = true;

  for (auto& kitchen : _processKitchens) {
    try {
      kitchen->sendMessage(Message(MessageType::KitchenClosing));
    } catch (...) {
    }
    kitchen->markClosing();
    kitchen->closePipes();
  }

  for (auto& kitchen : _processKitchens) {
    int status = 0;
    pid_t result = waitpid(kitchen->getPid(), &status, WNOHANG);
    if (result == 0) {
      kill(kitchen->getPid(), SIGTERM);
      waitpid(kitchen->getPid(), &status, 0);
    }
  }

  _processKitchens.clear();
}
