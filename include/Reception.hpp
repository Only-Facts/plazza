#pragma once

#include "Parser.hpp"
#include "KitchenManager.hpp"

class Reception {
public:
  Reception(double _multiplier, int cooksPerKitchen, int restockTime);
  ~Reception();

  void run();

private:
  void handleOrder(const std::string& input);
  void displayStatus();

  double _multiplier;
  int _cooksPerKitchen;
  int _restockTime;
  Parser _parser;
  KitchenManager _kitchenManager;
};
