#include "Message.hpp"

#include <sstream>
#include <stdexcept>

Message::Message()
  : _type(MessageType::Unknown),
    _pizza(PizzaType::Margarita, PizzaSize::S),
    _hasPizza(false)
{
}

Message::Message(MessageType type)
  : _type(type),
    _pizza(PizzaType::Margarita, PizzaSize::S),
    _hasPizza(false)
{
}

Message::Message(MessageType type, const Pizza& pizza)
  : _type(type),
    _pizza(pizza),
    _hasPizza(true)
{
}

MessageType Message::getType() const {
    return _type;
}

Pizza Message::getPizza() const {
  if (!_hasPizza)
    throw std::runtime_error("Message does not contain pizza");

  return _pizza;
}

bool Message::hasPizza() const {
  return _hasPizza;
}

std::string Message::messageTypeToString(MessageType type) {
  switch (type) {
    case MessageType::NewPizza:
      return "NEW_PIZZA";
    case MessageType::PizzaDone:
      return "PIZZA_DONE";
    case MessageType::StatusRequest:
      return "STATUS_REQUEST";
    case MessageType::StatusResponse:
      return "STATUS_RESPONSE";
    case MessageType::KitchenClosing:
      return "KITCHEN_CLOSING";
    case MessageType::Unknown:
      return "UNKNOWN";
  }
  return "UNKNOWN";
}

MessageType Message::stringToMessageType(const std::string& value) {
  if (value == "NEW_PIZZA")
    return MessageType::NewPizza;
  if (value == "PIZZA_DONE")
    return MessageType::PizzaDone;
  if (value == "STATUS_REQUEST")
    return MessageType::StatusRequest;
  if (value == "STATUS_RESPONSE")
    return MessageType::StatusResponse;
  if (value == "KITCHEN_CLOSING")
    return MessageType::KitchenClosing;
  return MessageType::Unknown;
}

std::string Message::pack() const {
  std::ostringstream stream;

  stream << messageTypeToString(_type);

  if (_hasPizza) {
    stream << "|"
            << static_cast<int>(_pizza.getType())
            << "|"
            << static_cast<int>(_pizza.getSize());
  }

  return stream.str();
}

Message Message::unpack(const std::string& data) {
  std::stringstream stream(data);
  std::string typePart;
  std::string pizzaTypePart;
  std::string pizzaSizePart;

  std::getline(stream, typePart, '|');

  MessageType messageType = stringToMessageType(typePart);

  if (messageType == MessageType::Unknown)
    throw std::invalid_argument("Unknown message type: " + typePart);

  if (!std::getline(stream, pizzaTypePart, '|'))
    return Message(messageType);

  if (!std::getline(stream, pizzaSizePart, '|'))
    throw std::invalid_argument("Invalid message format: missing pizza size");

  int pizzaTypeValue = std::stoi(pizzaTypePart);
  int pizzaSizeValue = std::stoi(pizzaSizePart);

  Pizza pizza(
    static_cast<PizzaType>(pizzaTypeValue),
    static_cast<PizzaSize>(pizzaSizeValue)
  );

  return Message(messageType, pizza);
}
