#pragma once
#include "Pizza.hpp"

#include <string>
#include <vector>


class Parser {
public:
  std::vector<Pizza> parseOrder(const std::string& input) const;
private:
  std::vector<std::string> splitCommands(const std::string& input) const;
  std::string trim(const std::string& str) const;
};
