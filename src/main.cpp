#include "Reception.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

static double parseMultiplier(const char* value)
{
    double multiplier = std::stod(value);

    if (multiplier <= 0)
        throw std::invalid_argument("Multiplier must be greater than 0");

    return multiplier;
}

static int parsePositiveInt(const char* value, const std::string& name)
{
    int number = std::stoi(value);

    if (number <= 0)
        throw std::invalid_argument(name + " must be greater than 0");

    return number;
}


int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./plazza multiplier cooks_per_kitchen restock_time" << std::endl;
    return 84;
  }

  try {
    double multiplier = parseMultiplier(argv[1]);
    int cooksPerKitchen = parsePositiveInt(argv[2], "Cooks per kitchen");
    int restockTime = parsePositiveInt(argv[3], "Restock time");

    Reception reception(multiplier, cooksPerKitchen, restockTime);
    reception.run();
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << std::endl;
    return 84;
  }
  return 0;
}
