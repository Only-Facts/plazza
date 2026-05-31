#pragma once

#include "Pizza.hpp"
#include "Stock.hpp"

#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

class ThreadPool {
public:
    ThreadPool(int cooksCount, double multiplier, Stock& stock, std::function<void(const Pizza&)> onPizzaDone);
    ~ThreadPool();

    void addTask(const Pizza& pizza);
    int getBusyCooks() const;
    int getWaitingTasks() const;
    void stop();

private:
    void workerLoop(int cookId);
    void cookPizza(int cookId, const Pizza& pizza);
    int getCookingTimeMs(const Pizza& pizza) const;
    void safePrint(const std::string& message) const;

    std::function<void(const Pizza&)> _onPizzaDone;

    int _cooksCount;
    double _multiplier;
    Stock& _stock;

    std::vector<std::thread> _workers;
    std::queue<Pizza> _tasks;

    mutable std::mutex _queueMutex;
    mutable std::mutex _coutMutex;
    std::condition_variable _condition;

    std::atomic<bool> _running;
    std::atomic<int> _busyCooks;
};
