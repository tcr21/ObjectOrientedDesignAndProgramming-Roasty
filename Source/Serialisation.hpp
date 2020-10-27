#pragma once

#include "Model/RoastyModel.hpp"
#include "Utilities.hpp"
#include <nlohmann/json.hpp>
#include <list>

nlohmann::json roastToJson(const Roast& r);
Roast jsonToRoast(nlohmann::json const& j, std::list<Bean> const&);

nlohmann::json eventToJson(const Event& e);
Event jsonToEvent(nlohmann::json const& j);

nlohmann::json ingredientToJson(const Ingredient& e);
Ingredient jsonToIngredient(nlohmann::json const& j, std::list<Bean> const&);

using namespace std::string_literals;

void to_json(nlohmann::json& j, const Bean& p);
template <typename T> void to_json(nlohmann::json& j, RangeGenerator<T> const& p) {
  j = nlohmann::json::array();
  for(auto& it : p) {
    j.push_back(it);
  }
}
void to_json(nlohmann::json& j, const Ingredient& p);
void to_json(nlohmann::json& j, const EventValue& p);

void to_json(nlohmann::json& j, const Event& p);
void to_json(nlohmann::json& j, const Roast& p);
