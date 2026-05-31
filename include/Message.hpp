#pragma once

#include "Pizza.hpp"
#include <string>

enum class MessageType {
  NewPizza,
  PizzaDone,
  StatusRequest,
  StatusResponse,
  KitchenClosing,
  Unknown
};

class Message {
public:
  Message();
  Message(MessageType type);
  Message(MessageType type, const Pizza& pizza);
  Message(MessageType type, const std::string& text);

  MessageType getType() const;
  Pizza getPizza() const;
  bool hasPizza() const;
  std::string getText() const;
  bool hasText() const;

  std::string pack() const;
  static Message unpack(const std::string& data);

private:
  static std::string messageTypeToString(MessageType type);
  static MessageType stringToMessageType(const std::string& value);
  static std::string escapeText(const std::string& text);
  static std::string unescapeText(const std::string& text);
  static PizzaType intToPizzaType(int value);
  static PizzaSize intToPizzaSize(int value);

  MessageType _type;
  Pizza _pizza;
  bool _hasPizza;
  std::string _text;
  bool _hasText;
};
