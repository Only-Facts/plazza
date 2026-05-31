#include "Reception.hpp"
#include "Message.hpp"

#include <iostream>
#include <string>

Reception::Reception(double multiplier, int cooksPerKitchen, int restockTime)
  : _multiplier(multiplier), _cooksPerKitchen(cooksPerKitchen), _restockTime(restockTime), _kitchenManager(cooksPerKitchen, multiplier, restockTime)
{
}

void Reception::run() {
  std::string input;

  std::cout << "Plazza Started." << std::endl;
  std::cout << "Type an order, status, or exit." << std::endl;

  while(true) {
    std::cout << "> ";

    if (!std::getline(std::cin, input))
      break;

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
    }
  }
}

void Reception::handleOrder(const std::string& input) {
  std::vector<Pizza> pizzas = _parser.parseOrder(input);

  std::cout << "Parsed " << pizzas.size() << " pizza(s)." << std::endl;

  for (const Pizza& pizza : pizzas)
    _kitchenManager.assignPizzaToProcess(pizza);
}

void Reception::displayStatus() const {
  std::cout << "Status:" << std::endl;
  std::cout << "- multiplier: " << _multiplier << std::endl;
  std::cout << "- cooks per kitchen: " << _cooksPerKitchen << std::endl;
  std::cout << "- restock time: " << _restockTime << " ms" << std::endl;
  _kitchenManager.displayStatus();
}
