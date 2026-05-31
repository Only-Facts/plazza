#include "Parser.hpp"

#include <sstream>
#include <stdexcept>
#include <cctype>
#include <string>

std::string Parser::trim(const std::string& str) const {
  std::size_t start = 0;
  std::size_t end = str.size();

  while(start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
    start++;
  while(end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
    end--;
  return str.substr(start, end - start);
}

std::vector<std::string> Parser::splitCommands(const std::string& input) const {
  std::vector<std::string> commands;
  std::stringstream stream(input);
  std::string command;

  while (std::getline(stream, command, ';')) {
    command = trim(command);
    if (!command.empty())
      commands.push_back(command);
  }
 
  return commands;
}

std::vector<Pizza> Parser::parseOrder(const std::string& input) const {
  std::vector<Pizza> pizzas;
  std::vector<std::string> commands = splitCommands(input);

  if (commands.empty())
    throw std::invalid_argument("Empty Order");

  for (const std::string& command: commands) {
    std::stringstream stream(command);

    std::string type;
    std::string size;
    std::string number;
    std::string extra;

    stream >> type >> size >> number;

    if (type.empty() || size.empty() || number.empty())
      throw::std::invalid_argument("Invalid Command: " + command);

    if (stream >> extra)
      throw::std::invalid_argument("Too many arguments in command: "+ command);

    if (number.size() < 2 || number[0] != 'x')
      throw::std::invalid_argument("Invalid number format: " + number);

    for (std::size_t i = 1; i < number.size(); i++)
      if (!std::isdigit(static_cast<unsigned char>(number[i])))
        throw::std::invalid_argument("Invalid number format: " + number);
    
    int count = std::stoi(number.substr(1));

    if (count <= 0)
      throw::std::invalid_argument("Pizza count must be positive");

    PizzaType pizzaType = Pizza::stringToType(type);
    PizzaSize pizzaSize = Pizza::stringToSize(size);

    for (int i = 0; i < count; i++)
      pizzas.emplace_back(pizzaType, pizzaSize);
  }
  return pizzas;
}
