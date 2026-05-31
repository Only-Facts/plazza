#include "Pizza.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

Pizza::Pizza(PizzaType type, PizzaSize size)
  : _type(type), _size(size)
{
}

PizzaType Pizza::getType() const
{
  return _type;
}

PizzaSize Pizza::getSize() const
{
  return _size;
}

std::string Pizza::typeToString() const
{
  switch (_type) {
    case PizzaType::Regina:
      return "regina";
    case PizzaType::Margarita:
      return "margarita";
    case PizzaType::Fantasia:
      return "fantasia";
    case PizzaType::Americana:
      return "americana";
  }
  return "unknown";
}

std::string Pizza::sizeToString() const
{
  switch (_size) {
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
  return "unknown";
}

PizzaType Pizza::stringToType(const std::string& value)
{
  std::string lowered = value;

  std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  if (lowered == "regina")
    return PizzaType::Regina;
  if (lowered == "margarita")
    return PizzaType::Margarita;
  if (lowered == "fantasia")
    return PizzaType::Fantasia;
  if (lowered == "americana")
    return PizzaType::Americana;
  throw std::invalid_argument("Invalid pizza type: " + value);
}

PizzaSize Pizza::stringToSize(const std::string& value)
{
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
