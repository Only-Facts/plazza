#include "Message.hpp"

#include <sstream>
#include <stdexcept>

Message::Message()
  : _type(MessageType::Unknown), _pizza(PizzaType::Margarita, PizzaSize::S), _hasPizza(false), _text(""), _hasText(false)
{
}

Message::Message(MessageType type)
  : _type(type), _pizza(PizzaType::Margarita, PizzaSize::S), _hasPizza(false), _text(""), _hasText(false)
{
}

Message::Message(MessageType type, const Pizza& pizza)
  : _type(type), _pizza(pizza), _hasPizza(true), _text(""), _hasText(false)
{
}

Message::Message(MessageType type, const std::string& text)
  : _type(type), _pizza(PizzaType::Margarita, PizzaSize::S), _hasPizza(false), _text(text), _hasText(true)
{
}

MessageType Message::getType() const
{
  return _type;
}

Pizza Message::getPizza() const
{
  if (!_hasPizza)
    throw std::runtime_error("Message does not contain pizza");
  return _pizza;
}

bool Message::hasPizza() const
{
  return _hasPizza;
}

std::string Message::getText() const
{
  return _text;
}

bool Message::hasText() const
{
  return _hasText;
}

std::string Message::messageTypeToString(MessageType type)
{
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

MessageType Message::stringToMessageType(const std::string& value)
{
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

PizzaType Message::intToPizzaType(int value)
{
  switch (value) {
    case static_cast<int>(PizzaType::Regina):
      return PizzaType::Regina;
    case static_cast<int>(PizzaType::Margarita):
      return PizzaType::Margarita;
    case static_cast<int>(PizzaType::Americana):
      return PizzaType::Americana;
    case static_cast<int>(PizzaType::Fantasia):
      return PizzaType::Fantasia;
  }
  throw std::invalid_argument("Invalid serialized pizza type");
}

PizzaSize Message::intToPizzaSize(int value)
{
  switch (value) {
    case static_cast<int>(PizzaSize::S):
      return PizzaSize::S;
    case static_cast<int>(PizzaSize::M):
      return PizzaSize::M;
    case static_cast<int>(PizzaSize::L):
      return PizzaSize::L;
    case static_cast<int>(PizzaSize::XL):
      return PizzaSize::XL;
    case static_cast<int>(PizzaSize::XXL):
      return PizzaSize::XXL;
  }
  throw std::invalid_argument("Invalid serialized pizza size");
}

std::string Message::escapeText(const std::string& text)
{
  std::string escaped;

  for (char c : text) {
    if (c == '\\')
      escaped += "\\\\";
    else if (c == '\n')
      escaped += "\\n";
    else
      escaped += c;
  }
  return escaped;
}

std::string Message::unescapeText(const std::string& text)
{
  std::string unescaped;

  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\\' && i + 1 < text.size()) {
      if (text[i + 1] == 'n') {
        unescaped += '\n';
        ++i;
      } else if (text[i + 1] == '\\') {
        unescaped += '\\';
        ++i;
      } else {
        unescaped += text[i];
      }
    } else {
      unescaped += text[i];
    }
  }
  return unescaped;
}

std::string Message::pack() const
{
  std::ostringstream stream;

  stream << messageTypeToString(_type);
  if (_hasPizza) {
    stream << "|PIZZA|" << static_cast<int>(_pizza.getType()) << "|" << static_cast<int>(_pizza.getSize());
  } else if (_hasText) {
    stream << "|TEXT|" << escapeText(_text);
  }
  return stream.str();
}

Message Message::unpack(const std::string& data)
{
  std::stringstream stream(data);
  std::string typePart;
  std::string mode;

  if (!std::getline(stream, typePart, '|'))
    throw std::invalid_argument("Empty message");

  MessageType messageType = stringToMessageType(typePart);
  if (messageType == MessageType::Unknown)
    throw std::invalid_argument("Unknown message type: " + typePart);

  if (!std::getline(stream, mode, '|'))
    return Message(messageType);

  if (mode == "PIZZA") {
    std::string pizzaTypePart;
    std::string pizzaSizePart;

    if (!std::getline(stream, pizzaTypePart, '|') || !std::getline(stream, pizzaSizePart, '|'))
      throw std::invalid_argument("Invalid pizza message");

    int pizzaTypeValue = std::stoi(pizzaTypePart);
    int pizzaSizeValue = std::stoi(pizzaSizePart);

    return Message(messageType, Pizza(intToPizzaType(pizzaTypeValue), intToPizzaSize(pizzaSizeValue)));
  }

  if (mode == "TEXT") {
    std::string text;
    std::getline(stream, text);
    return Message(messageType, unescapeText(text));
  }

  throw std::invalid_argument("Invalid message payload mode: " + mode);
}
