#include "Kitchen.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

Kitchen::Kitchen(int id, int cooksCount, double multiplier, int restockTime)
  : _id(id),
    _cooksCount(cooksCount),
    _multiplier(multiplier),
    _restockTime(restockTime),
    _maxCapacity(cooksCount * 2),
    _stock(),
    _threadPool(cooksCount, multiplier, _stock),
    _running(true)
{
  _stockThread = std::thread(&Kitchen::stockRegenerationLoop, this);
}

Kitchen::~Kitchen() {
  _running = false;

  if (_stockThread.joinable())
    _stockThread.join();
}

void Kitchen::stockRegenerationLoop() {
  while (_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(_restockTime));

    if (!_running)
      break;

    _stock.regenerate();
  }
}

int Kitchen::getCurrentLoad() const {
  return _threadPool.getBusyCooks() + _threadPool.getWaitingTasks();
}

int Kitchen::getLoad() const {
  return getCurrentLoad();
}

int Kitchen::getCapacity() const {
  return _maxCapacity;
}

int Kitchen::getId() const {
  return _id;
}

bool Kitchen::canAcceptPizza() const {
  return getCurrentLoad() < _maxCapacity;
}

void Kitchen::addPizza(const Pizza& pizza) {
  if (!canAcceptPizza())
    throw std::runtime_error("Kitchen is full");

  std::cout << "Kitchen " << _id
              << " received "
              << pizza.typeToString()
              << " "
              << pizza.sizeToString()
              << std::endl;

  _threadPool.addTask(pizza);
}

void Kitchen::displayStatus() const {
  std::cout << "Kitchen " << _id << ":" << std::endl;
  std::cout << "- cooks: " << _cooksCount << std::endl;
  std::cout << "- busy cooks: " << _threadPool.getBusyCooks() << "/" << _cooksCount << std::endl;
  std::cout << "- waiting pizzas: " << _threadPool.getWaitingTasks() << std::endl;
  std::cout << "- load: " << getCurrentLoad() << "/" << _maxCapacity << std::endl;
  std::cout << "- restock time: " << _restockTime << " ms" << std::endl;
  _stock.display();
}
