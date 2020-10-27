#include "Roasty.hpp"
#include "Storage/DiskStorage.hpp"
#include "Storage/MemoryStorage.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

template <typename StorageImplementation> void Roasty<StorageImplementation>::startServer() {
  roastyServer.startServer();
}

template <typename RoastyImplementation> std::list<Bean>& Roasty<RoastyImplementation>::allBeans() {
  return storage.getBeans();
}

auto const check_unique = [](auto& container, auto comparator) {
  auto it = std::find_if(container.begin(), container.end(), comparator);
  return it == container.end();
};

template <typename RoastyImplementation> void Roasty<RoastyImplementation>::addBean(Bean&& bean) {
  auto& allBean = storage.getBeans();

  if(!check_unique(allBean, [&bean](const auto& b) { return b.getName() == bean.getName(); })) {
    throw std::runtime_error{"Cannot add bean, they already exist."};
  }
  storage.addBean(std::move(bean));
}

template <typename RoastyImplementation>
Bean& Roasty<RoastyImplementation>::getBeanByName(std::string const& name) {
  return *std::find_if(begin(storage.getBeans()), end(storage.getBeans()),
                       [&name](auto const& bean) { return bean.getName() == name; });
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::deleteBean(const Bean& bean) {
  storage.getBeans().remove_if(
      [name = bean.getName()](auto const& bean) { return bean.getName() == name; });
}

template <typename RoastyImplementation>
std::list<Roast>& Roasty<RoastyImplementation>::allRoasts() {
  return storage.getRoasts();
}

template <typename RoastyImplementation> Roast& Roasty<RoastyImplementation>::getRoast(long id) {
  auto& allRoasts = storage.getRoasts();

  auto it = std::find_if(allRoasts.begin(), allRoasts.end(),
                         [id](auto& elem) { return elem.getTimestamp() == id; });

  if(it == allRoasts.end()) {
    throw std::runtime_error{"Unknown roast id"};
  }

  return *it;
}

template <typename RoastyImplementation>
Roast& Roasty<RoastyImplementation>::addRoast(Roast&& roast) {
  auto& roasts = storage.getRoasts();
  for(auto& it : roasts) {
    if(it.getTimestamp() == roast.getTimestamp()) {
      throw std::runtime_error{"Cannot add roast, id already exists."};
    }
  }
  return roasts.emplace_back(roast);
}

template <typename RoastyImplementation> void Roasty<RoastyImplementation>::deleteRoast(long id) {
  allRoasts().remove_if([id](auto& elem) { return elem.getTimestamp() == id; });
}

template <typename RoastyImplementation>
Ingredient const&
Roasty<RoastyImplementation>::getIngredientByBeanName(long roastId, std::string const& beanName) {
  auto& roast = getRoast(roastId);

  auto ingredients = RangeGenerator<const Ingredient>(
      [&](auto i) -> Ingredient const& { return roast.getIngredient(i); },
      roast.getIngredientsCount());

  auto it =
      std::find_if(ingredients.begin(), ingredients.end(), [&beanName](const auto& ingredient) {
        return ingredient.getBean().getName() == beanName;
      });
  if(it == ingredients.end()) {
    std::stringstream message{};
    message << "No bean with name " << beanName << " in roast " << roastId;
    throw std::runtime_error(message.str());
  }

  return *it;
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::addIngredientToRoast(long roastId, Ingredient&& ingredient) {
  auto& roast = getRoast(roastId);

  auto ingredients = RangeGenerator<const Ingredient>(
      [&](auto i) -> Ingredient const& { return roast.getIngredient(i); },
      roast.getIngredientsCount());

  if(!check_unique(ingredients, [&ingredient](const auto& b) {
       return b.getBean().getName() == ingredient.getBean().getName();
     })) {
    throw std::runtime_error{"Cannot add ingredient, ingredient already exists."};
  }
  roast.addIngredient(std::move(ingredient));
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::removeIngredientFromRoast(long roastId,
                                                             std::string const& beanName) {
  auto& roast = getRoast(roastId);
  roast.removeIngredientByBeanName(beanName);
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::updateIngredient(long roastId, std::string const& beanName,
                                                    int newAmount) {
  auto& roast = getRoast(roastId);
  roast.removeIngredientByBeanName(beanName);
  roast.addIngredient({getBeanByName(beanName), newAmount});
}

template <typename RoastyImplementation>
Event const& Roasty<RoastyImplementation>::getEventById(long roastId, long eventTimestamp) {
  auto& roast = getRoast(roastId);
  auto events = RangeGenerator<const Event>(
      [&](auto i) -> Event const& { return roast.getEvent(i); }, roast.getEventCount());

  auto it = std::find_if(events.begin(), events.end(), [eventTimestamp](const auto& event) {
    return event.getTimestamp() == eventTimestamp;
  });

  if(it == events.end()) {
    std::stringstream message{};
    message << "No event with timestamp " << eventTimestamp << " in roast" << roastId;
    throw std::runtime_error(message.str());
  }

  return *it;
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::addEventToRoast(long roastId, Event&& e) {
  auto& roast = getRoast(roastId);
  for(auto i = 0U; i < roast.getEventCount(); i++) {
    if(roast.getEvent(i).getTimestamp() == e.getTimestamp()) {
      throw std::runtime_error{"Cannot add event, id already exists."};
    }
  }
  roast.addEvent(std::move(e));
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::removeEventFromRoast(long roastId, long eventTimestamp) {
  auto& roast = getRoast(roastId);
  roast.removeEventByTimestamp(eventTimestamp);
}

template <typename RoastyImplementation>
void Roasty<RoastyImplementation>::replaceEventInRoast(long roastId, long oldEventTimestamp,
                                                       Event&& newEvent) {
  auto& roast = getRoast(roastId);
  roast.removeEventByTimestamp(oldEventTimestamp);
  roast.addEvent(std::move(newEvent));
}

template struct Roasty<MemoryStorage>;
template struct Roasty<DiskStorage>;
