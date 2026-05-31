#include "Pizza.hpp"

#include <stdexcept>

Pizza::Pizza(PizzaType type, PizzaSize size)
: _type(type), _size(size)
{
}

PizzaType Pizza::getType() const {
  return _type;
}

PizzaSize Pizza::getSize() const {
  return _size;
}

std::string Pizza::typeToString() const {
  switch(_type) {
    case PizzaType::Regina:
      return "regina";
    case PizzaType::Margarita:
      return "margarita";
    case PizzaType::Fantasia:
      return "fantasia";
    case PizzaType::Americana:
      return "americana";
  }
  return "unknow";
};

std::string Pizza::sizeToString() const {
  switch(_size) {
    case PizzaSize::S:
      return "S";
    case PizzaSize::M:
      return "M";
    case PizzaSize::L:
      return "L";
    case PizzaSize::XL:
      return "XL";
    case PizzaSize::XXL:
      return "XXL";
  }
  return "unknow";
}

PizzaType Pizza::stringToType(const std::string& value) {
  if (value == "regina")
    return PizzaType::Regina;
  if (value == "margarita")
    return PizzaType::Margarita;
  if (value == "fantasia")
    return PizzaType::Fantasia;
  if (value == "americana")
    return PizzaType::Americana;
  throw std::invalid_argument("Invalid pizza type:" + value);
}

PizzaSize Pizza::stringToSize(const std::string& value) {
    if (value == "S")
        return PizzaSize::S;
    if (value == "M")
        return PizzaSize::M;
    if (value == "L")
        return PizzaSize::L;
    if (value == "XL")
        return PizzaSize::XL;
    if (value == "XXL")
        return PizzaSize::XXL;

    throw std::invalid_argument("Invalid pizza size: " + value);
}
