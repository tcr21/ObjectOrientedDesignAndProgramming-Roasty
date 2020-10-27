#pragma once
#include "../Model/RoastyModel.hpp"
#include <list>
#include <utility>

#include "../Utilities.hpp"
#include <nlohmann/json.hpp>

class MemoryStorage {
  std::list<Bean> beans;
  std::list<Roast> roasts;

public:
  std::list<Bean>& getBeans() { return beans; }
  void addBean(Bean&& b) { beans.emplace_back(std::move(b)); };

  std::list<Roast>& getRoasts() { return roasts; }
  virtual void commit() {}
};
