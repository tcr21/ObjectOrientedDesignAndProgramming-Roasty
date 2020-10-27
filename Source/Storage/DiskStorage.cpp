#include "DiskStorage.hpp"
#include "../Serialisation.hpp"
#include <exception>
#include <iostream>
#include <string>

static auto const IOdebug = false;

DiskStorage::DiskStorage() {
  auto data = readJson(beansJsonFileName);
  auto& beanStrings = data["beans"];

  try {
    for(auto& bean : beanStrings) {
      auto value = bean.get<std::string>();
      getBeans().emplace_back(Bean{value});
    }
  } catch(std::exception& e) {
    std::stringstream message{};
    message << "Corrupt database file bean.json! Error while reading: " << e.what();
    throw std::runtime_error{message.str()};
  }

  auto json = readJson(STRINGIZE(ROASTY_SOURCE_DIR) "/roasts.json");

  try {
    for(auto const& roastJ : json) {
      getRoasts().push_back(jsonToRoast(roastJ, getBeans()));
    }

  } catch(std::exception& e) {
    std::stringstream message{};
    message << "Corrupt database file roasts.json! Error while reading: " << e.what();
    throw std::runtime_error{message.str()};
  }
}

json DiskStorage::readJson(std::string const& file) {
  std::ifstream i(file);

  if(i.fail()) {
    return {};
  }

  try {
    json j;
    i >> j;
    return j;
  } catch(std::exception& e) {
    throw std::runtime_error("Corrupt database file");
  }
}

void DiskStorage::writeJson(std::string const& file, json const& j) {
  std::ofstream o(file);

  if(o.fail()) {
    throw std::runtime_error{"Error reading database file"};
  }

  if(j.empty()) {
    o << json::array();
  } else {
    o << j.dump(2);
  }
}

void DiskStorage::commit() {
  writeJson(beansJsonFileName, json({{"beans", getBeans()}}));
  writeJson(roastsJsonFileName, json(getRoasts()));
}
