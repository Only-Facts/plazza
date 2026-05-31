#pragma once

#include "KitchenProcess.hpp"
#include "Pizza.hpp"

#include <memory>
#include <vector>

class KitchenManager {
public:
  KitchenManager(int cooksPerKitchen, double multiplier, int restockTime);
  ~KitchenManager();

  KitchenManager(const KitchenManager&) = delete;
  KitchenManager& operator=(const KitchenManager&) = delete;

  void assignPizzaToProcess(const Pizza& pizza);
  void displayStatus();
  void update();
  void shutdown();

private:
  KitchenProcess& createKitchenProcess();
  KitchenProcess& getBestKitchenProcess();
  void removeClosedProcesses();
  void cleanupDeadProcesses();
  bool handleKitchenMessage(std::size_t index);

  int _cooksPerKitchen;
  double _multiplier;
  int _restockTime;
  int _nextKitchenId;
  bool _shuttingDown;

  std::vector<std::unique_ptr<KitchenProcess>> _processKitchens;
};
