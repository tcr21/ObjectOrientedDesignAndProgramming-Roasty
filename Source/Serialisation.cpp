#include "Serialisation.hpp"
#include <algorithm>
#include <exception>
#include <functional>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

template <typename T> T parseWithErrorHandling(std::function<T()> action) {
  try {
    return action();
  } catch(json::parse_error& e) {
    throw std::runtime_error(e.what());
  } catch(json::type_error& e) {
    throw std::runtime_error(e.what());
  } catch(std::exception& e) {
    throw std::runtime_error(e.what());
  }
}

json roastToJson(const Roast& r) {
  json roast{{"timestamp", r.getTimestamp()}};

  for(auto i = 0U; i < r.getEventCount(); i++) {
    roast["events"].push_back(eventToJson(r.getEvent(i)));
  }

  for(auto i = 0; i < r.getIngredientsCount(); i++) {
    roast["beans"].push_back(ingredientToJson(r.getIngredient(i)));
  }

  return roast;
}

Roast jsonToRoast(json const& j, std::list<Bean> const& beans) {
  return parseWithErrorHandling<Roast>([&] {
    auto roast = Roast{j.at("timestamp").get<long>()};

    for(auto const& event : j.at("events")) {
      roast.addEvent(jsonToEvent(event));
    }

    for(auto const& blend : j.at("beans")) {
      roast.addIngredient(jsonToIngredient(blend, beans));
    }

    return roast;
  });
}

json eventToJson(const Event& e) {
  // clang-format off
  json j{
    {"id", e.getTimestamp()},
    {"timestamp", e.getTimestamp()},
    {"type", e.getType()}
  };
  // clang-format on

  if(e.hasValue()) {
    j["value"] = e.getValue()->getValue();
  }

  return j;
}

Event jsonToEvent(json const& j) {
  return parseWithErrorHandling<Event>([&]() -> Event {
    EventValue* eventValue = nullptr;

    if(!j.at("value").empty()) {
      auto value = j.at("value").get<long>();
      eventValue = new EventValue(value);
    }
    auto type = j.at("type").get<std::string>();
    auto timestamp = j.at("timestamp").get<long>();
    return {type, timestamp, eventValue};
  });
}

nlohmann::json ingredientToJson(const Ingredient& e) {
  // clang-format off
  return json{
    {"name", e.getBean().getName()},
    {"amount", e.getAmount()}
  };
  // clang-format on
}

Ingredient jsonToIngredient(nlohmann::json const& j, std::list<Bean> const& beans) {
  return parseWithErrorHandling<Ingredient>([&] {
    auto found = std::find_if(begin(beans), end(beans),
                              [name = j.at("name").get<std::string>()](auto const& bean) {
                                return bean.getName() == name;
                              });
    if(found == end(beans))
      throw std::runtime_error(std::string("no bean with name ") + j.at("name").get<std::string>() +
                               " exists in input list");
    return Ingredient{*found, j.at("amount").get<int>()};
  });
}

void to_json(nlohmann::json& j, const Bean& p) { j = nlohmann::json(p.getName()); }
void to_json(nlohmann::json& j, const Ingredient& p) {
  j = nlohmann::json{{"name", p.getBean()}, {"amount", p.getAmount()}};
}
void to_json(nlohmann::json& j, const Event& p) {
  j = nlohmann::json{
      {"id", p.getTimestamp()},
      {"timestamp", p.getTimestamp()},
      {"type", p.getType()},
  };
  if(p.hasValue()) {
    j["value"] = *p.getValue();
  }
}
void to_json(nlohmann::json& j, const Roast& p) {
  j["timestamp"] = p.getTimestamp();
  j["beans"] = json::array();
  j["events"] = json::array();
  for(auto i = 0; i < p.getIngredientsCount(); i++) {
    j["beans"].emplace_back(p.getIngredient(i));
  }
  for(auto i = 0; i < p.getEventCount(); i++) {
    j["events"].push_back(p.getEvent(i));
  }
}
void to_json(nlohmann::json& j, const EventValue& p) { j = nlohmann::json(p.getValue()); }
