#pragma once

#include "Pizza.hpp"
#include "Stock.hpp"
#include "ThreadPool.hpp"

class Kitchen {
public:
  Kitchen(int id, int cooksCount, double multiplier, int restockTime);
  ~Kitchen();
  bool canAcceptPizza() const;
  void addPizza(const Pizza& pizza);

  int getLoad() const;
  int getCapacity() const;
  int getId() const;

  void displayStatus() const;
private:
  int getCurrentLoad() const;
  void stockRegenerationLoop();

  int _id;
  int _cooksCount;
  double _multiplier;
  int _restockTime;
  int _currentLoad;
  int _maxCapacity;

  Stock _stock;
  ThreadPool _threadPool;

  std::thread _stockThread;
  std::atomic<bool> _running;
  mutable std::mutex _stockDisplayMutex;
};
