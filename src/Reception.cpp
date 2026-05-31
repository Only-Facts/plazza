#include "Reception.hpp"
#include "Logger.hpp"

#include <cerrno>
#include <iostream>
#include <poll.h>
#include <string>
#include <unistd.h>

Reception::Reception(double multiplier, int cooksPerKitchen, int restockTime)
  : _multiplier(multiplier),
    _cooksPerKitchen(cooksPerKitchen),
    _restockTime(restockTime),
    _kitchenManager(cooksPerKitchen, multiplier, restockTime)
{
}

Reception::~Reception()
{
  _kitchenManager.shutdown();
}

void Reception::run()
{
  std::string input;

  Logger::log("Plazza started");
  std::cout << "Plazza Started." << std::endl;
  std::cout << "Type an order, status, or exit." << std::endl;

  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN | POLLHUP | POLLERR;
  pfd.revents = 0;

  bool printPrompt = true;

  while (true) {
    if (printPrompt) {
      std::cout << "> " << std::flush;
      printPrompt = false;
    }

    _kitchenManager.update();

    pfd.revents = 0;
    int ret = poll(&pfd, 1, 50);
    if (ret < 0) {
      if (errno == EINTR)
        continue;
      break;
    }

    if (ret > 0 && (pfd.revents & (POLLIN | POLLHUP | POLLERR))) {
      if (!std::getline(std::cin, input))
        break;

      printPrompt = true;

      if (input.empty())
        continue;
      if (input == "exit")
        break;
      if (input == "status") {
        displayStatus();
        continue;
      }

      try {
        handleOrder(input);
      } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        Logger::log(std::string("Order error: ") + error.what());
      }
    }
  }

  Logger::log("Plazza stopped");
}

void Reception::handleOrder(const std::string& input)
{
  std::vector<Pizza> pizzas = _parser.parseOrder(input);

  std::cout << "Parsed " << pizzas.size() << " pizza(s)." << std::endl;
  for (const Pizza& pizza : pizzas)
    _kitchenManager.assignPizzaToProcess(pizza);
}

void Reception::displayStatus()
{
  std::cout << "Status:" << std::endl;
  std::cout << "- multiplier: " << _multiplier << std::endl;
  std::cout << "- cooks per kitchen: " << _cooksPerKitchen << std::endl;
  std::cout << "- restock time: " << _restockTime << " ms" << std::endl;
  _kitchenManager.displayStatus();
}
