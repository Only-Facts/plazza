#pragma once

#include "Pizza.hpp"

#include <map>
#include <string>
#include <vector>
#include <mutex>

enum class Ingredient {
  Dough,
  Tomato,
  Gruyere,
  Ham,
  Mushrooms,
  Steak,
  Eggplant,
  GoatCheese,
  ChiefLove
};

class Stock {
public:
  Stock();

  bool hasIngredients(const Pizza& pizza) const;
  bool tryConsumeIngredients(const Pizza& pizza);
  void consumeIngredients(const Pizza& pizza);
  void regenerate();
  void display() const;
private:
  std::vector<Ingredient> getIngredientsForPizza(const Pizza& pizza) const;
  std::string ingredientToString(Ingredient ingredient) const;
  bool hasIngredientsUnlocked(const Pizza& pizza) const;

  std::map<Ingredient, int> _ingredients;
  mutable std::mutex _mutex;
};
