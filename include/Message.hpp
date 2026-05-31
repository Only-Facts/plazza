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

  MessageType getType() const;
  Pizza getPizza() const;
  bool hasPizza() const;

  std::string pack() const;
  static Message unpack(const std::string& data);

private:
  static std::string messageTypeToString(MessageType type);
  static MessageType stringToMessageType(const std::string& value);

  MessageType _type;
  Pizza _pizza;
  bool _hasPizza;
};
