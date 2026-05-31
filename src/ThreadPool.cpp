#include "ThreadPool.hpp"
#include "Pizza.hpp"

#include <iostream>
#include <chrono>
#include <mutex>
#include <string>

ThreadPool::ThreadPool(int cooksCount, double multiplier, Stock& stock)
  : _cooksCount(cooksCount),
    _multiplier(multiplier),
    _stock(stock),
    _running(true),
    _busyCooks(0)
{
  for (int i = 0; i < _cooksCount; i++)
    _workers.emplace_back(&ThreadPool::workerLoop, this, i + 1);
}

ThreadPool::~ThreadPool()
{
  stop();
}

void ThreadPool::stop() {
  if (!_running)
    return;

  _running = false;
  _condition.notify_all();

  for(std::thread& worker : _workers)
    if (worker.joinable())
      worker.join();
}

void ThreadPool::addTask(const Pizza& pizza) {
  {
    std::lock_guard<std::mutex> lock(_queueMutex);
    _tasks.push(pizza);
  }
  _condition.notify_one();
}

void ThreadPool::safePrint(const std::string& message) const {
  std::lock_guard<std::mutex> lock(_coutMutex);
  std::cout << message << std::endl;
}

int ThreadPool::getBusyCooks() const {
  return _busyCooks.load();
}

int ThreadPool::getWaitingTasks() const {
  std::lock_guard<std::mutex> lock(_queueMutex);
  return static_cast<int>(_tasks.size());
}

void ThreadPool::workerLoop(int cookId) {
  while(true){
    Pizza pizza(PizzaType::Margarita, PizzaSize::S);
    {
      std::unique_lock<std::mutex> lock(_queueMutex);
      _condition.wait(lock, [this](){
        return !_tasks.empty() || !_running;
      });

      if (!_running && _tasks.empty())
        return;

      pizza = _tasks.front();
      _tasks.pop();
      _busyCooks++;
    }
    cookPizza(cookId, pizza);
    _busyCooks--;
  }
}

void ThreadPool::cookPizza(int cookId, const Pizza& pizza) {
  if (!_stock.tryConsumeIngredients(pizza)) {
    safePrint(
      "Cook " + std::to_string(cookId)
      + " cannot cook "
      + pizza.typeToString()
      + " "
      + pizza.sizeToString()
      + ": not enough ingredients"
    );
    return;
  }

  safePrint(
    "Cook " + std::to_string(cookId)
    + " is cooking "
    + pizza.typeToString()
    + " "
    + pizza.sizeToString()
  );

  std::this_thread::sleep_for(std::chrono::milliseconds(getCookingTimeMs(pizza)));

  safePrint(
    "Cook " + std::to_string(cookId)
    + " finished "
    + pizza.typeToString()
    + " "
    + pizza.sizeToString()
  );
}

int ThreadPool::getCookingTimeMs(const Pizza& pizza) const {
  int baseTime = 0;

  switch (pizza.getType()) {
    case PizzaType::Margarita:
      baseTime = 1000;
      break;
    case PizzaType::Regina:
      baseTime = 2000;
      break;
    case PizzaType::Americana:
      baseTime = 2000;
      break;
    case PizzaType::Fantasia:
      baseTime = 4000;
      break;
  }
  return static_cast<int>(baseTime * _multiplier);
}
