#pragma once

#include "../Model/RoastyModel.hpp"
#include "../Utilities.hpp"
#include "MemoryStorage.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

using json = nlohmann::json;

class DiskStorage : public MemoryStorage {
public:
  DiskStorage();
  void commit() override ;

private:
  std::string const beansJsonFileName = STRINGIZE(ROASTY_SOURCE_DIR) "/beans.json";
  std::string const roastsJsonFileName = STRINGIZE(ROASTY_SOURCE_DIR) "/roasts.json";
  static json readJson(std::string const& file);
  static void writeJson(std::string const& file, json const& j);
};
