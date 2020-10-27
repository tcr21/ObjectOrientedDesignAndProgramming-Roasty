#pragma once

#include "Model/RoastyModel.hpp"
#include "Server/RoastyServer.hpp"
#include <list>

template <typename StorageImplementation> struct Roasty {
public:
  void startServer();

  // ============== Bean =================
  std::list<Bean>& allBeans();
  void addBean(Bean&& bean);
  void deleteBean(Bean const& bean);
  Bean& getBeanByName(std::string const& name);

  // ============== Roasts ================
  std::list<Roast>& allRoasts();
  Roast& getRoast(long id);
  Roast& addRoast(Roast&& r);
  void deleteRoast(long id);

  // ============== Ingredients ================
  Ingredient const& getIngredientByBeanName(long roastId, std::string const& beanName);
  void addIngredientToRoast(long roastId, Ingredient&& ingredient);
  void removeIngredientFromRoast(long roastId, std::string const& beanName);
  void updateIngredient(long roastId, std::string const& beanName, int newAmount);

  // ============== Events ================
  Event const& getEventById(long roastId, long eventId);
  void addEventToRoast(long roastId, Event&& e);
  void removeEventFromRoast(long roastId, long eventTimestamp);
  void replaceEventInRoast(long roastId, long oldEventTimestamp, Event&& newEvent);
  void commit() { storage.commit(); }

private:
  RoastyServer<Roasty<StorageImplementation>> roastyServer;
  StorageImplementation storage;

  template <typename StorageImpl> friend StorageImpl& peakIntoStorage(Roasty<StorageImpl>&);
};
