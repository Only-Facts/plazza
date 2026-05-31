#pragma once
#include <string>

enum class PizzaType {
    Regina = 1,
    Margarita = 2,
    Americana = 4,
    Fantasia = 8
};

enum class PizzaSize {
    S = 1,
    M = 2,
    L = 4,
    XL = 8,
    XXL = 16
};

class Pizza {
public:
  Pizza(PizzaType type, PizzaSize size);
  PizzaType getType() const;
  PizzaSize getSize() const;

  std::string typeToString() const;
  std::string sizeToString() const;

  static PizzaType stringToType(const std::string& value);
  static PizzaSize stringToSize(const std::string& value);

private:
  PizzaType _type;
  PizzaSize _size;
};
