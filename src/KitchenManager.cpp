#include "KitchenManager.hpp"
#include "Kitchen.hpp"
#include "PipeIPC.hpp"
#include "Message.hpp"

#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>

KitchenManager::KitchenManager(int cooksPerKitchen, double multiplier, int restockTime)
  : _cooksPerKitchen(cooksPerKitchen),
    _multiplier(multiplier),
    _restockTime(restockTime),
    _nextKitchenId(1)
{
  createKitchen();
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
  Kitchen kitchen(kitchenId, cooksPerKitchen, multiplier, restockTime);

  while (true) {
    try {
      std::string rawMessage = fromReception.receive();
      Message message = Message::unpack(rawMessage);

      if (message.getType() == MessageType::NewPizza && message.hasPizza()) {
        Pizza pizza = message.getPizza();

        std::cout << "Kitchen process " << kitchenId
                  << " received "
                  << pizza.typeToString()
                  << " "
                  << pizza.sizeToString()
                  << std::endl;

        kitchen.addPizza(pizza);

        Message doneMessage(MessageType::PizzaDone, pizza);
        toReception.send(doneMessage.pack());
      }
    } catch (const std::exception& error) {
      std::cerr << "Kitchen process " << kitchenId
                << " stopped: "
                << error.what()
                << std::endl;
      break;
    }
  }

  std::exit(0);
}

KitchenProcess& KitchenManager::createKitchenProcess() {
  int pipeToChild[2];
  int pipeToParent[2];

  if (pipe(pipeToChild) == -1)
    throw std::runtime_error("pipe to child failed");

  if (pipe(pipeToParent) == -1)
    throw std::runtime_error("pipe to parent failed");

  int id = _nextKitchenId++;
  int capacity = _cooksPerKitchen * 2;

  pid_t pid = fork();

  if (pid == -1)
    throw std::runtime_error("fork failed");

  if (pid == 0) {
    PipeIPC fromReception(pipeToChild[0], -1);
    PipeIPC toReception(-1, pipeToParent[1]);

    close(pipeToChild[1]);
    close(pipeToParent[0]);

    runKitchenChild(
    id,
    _cooksPerKitchen,
    _multiplier,
    _restockTime,
    std::move(fromReception),
    std::move(toReception)
    );
  }

  PipeIPC toKitchen(-1, pipeToChild[1]);
  PipeIPC fromKitchen(pipeToParent[0], -1);

  close(pipeToChild[0]);
  close(pipeToParent[1]);

  _processKitchens.emplace_back(
    id,
    pid,
    std::move(toKitchen),
    std::move(fromKitchen),
    capacity
  );

  std::cout << "Kitchen process " << id
            << " created with pid "
            << pid
            << std::endl;

  return _processKitchens.back();
}

KitchenProcess& KitchenManager::getBestKitchenProcess() {
  KitchenProcess* bestKitchen = nullptr;

  for (KitchenProcess& kitchen : _processKitchens) {
    if (!kitchen.canAcceptPizza())
      continue;

    if (bestKitchen == nullptr || kitchen.getLoad() < bestKitchen->getLoad())
      bestKitchen = &kitchen;
  }

  if (bestKitchen == nullptr)
    return createKitchenProcess();

  return *bestKitchen;
}

void KitchenManager::assignPizzaToProcess(const Pizza& pizza) {
  KitchenProcess& kitchen = getBestKitchenProcess();

  std::cout << "Sending "
            << pizza.typeToString()
            << " "
            << pizza.sizeToString()
            << " to kitchen process "
            << kitchen.getId()
            << std::endl;

  kitchen.sendPizza(pizza);
  kitchen.incrementLoad();

  Pizza donePizza(PizzaType::Margarita, PizzaSize::S);

  if (kitchen.receiveDonePizza(donePizza)) {
    kitchen.decrementLoad();

    std::cout << "Reception received done pizza from kitchen process "
              << kitchen.getId()
              << ": "
              << donePizza.typeToString()
              << " "
              << donePizza.sizeToString()
              << std::endl;
  }
}


void KitchenManager::createKitchen() {
  int id = _nextKitchenId++;

  _kitchens.push_back(
    std::make_unique<Kitchen>(
        id,
        _cooksPerKitchen,
        _multiplier,
        _restockTime
    )
  );
  std::cout << "Kitchen " << id << " created" << std::endl;
}

Kitchen& KitchenManager::getBestKitchen() {
  Kitchen* bestKitchen = nullptr;

  for (const auto& kitchen : _kitchens) {
    if (!kitchen->canAcceptPizza())
      continue;

    if (bestKitchen == nullptr || kitchen->getLoad() < bestKitchen->getLoad())
      bestKitchen = kitchen.get();
  }

  if (bestKitchen == nullptr) {
    createKitchen();
    return *_kitchens.back();
  }

  return *bestKitchen;
}

void KitchenManager::assignPizza(const Pizza& pizza) {
  Kitchen& kitchen = getBestKitchen();

  kitchen.addPizza(pizza);
}

void KitchenManager::displayStatus() const {
  for (const auto& kitchen : _kitchens)
    kitchen->displayStatus();
}
