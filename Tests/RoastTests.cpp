#include "../Source/Model/RoastyModel.hpp"
#include "../Source/Utilities.hpp"
#include <catch2/catch.hpp>
#include <list>
#include <unordered_set>
#include <vector>

TEST_CASE("Ingredient Count is Correct") {
  auto timestamp = 12345;
  auto beans = std::list<Bean>();
  Roast r{timestamp};

  for(auto i = 0; i < 20; i++) {
    auto amount = i * 10;
    r.addIngredient(Ingredient{beans.emplace_back("Elimbari" + std::to_string(i)), amount});
  }

  REQUIRE(r.getIngredientsCount() == 20);
  auto ingredients = RangeGenerator<Ingredient const>(
      [&r](size_t i) -> Ingredient const& { return r.getIngredient(i); }, r.getIngredientsCount());
  auto amounts = std::unordered_set<int>();
  std::transform(std::begin(ingredients), std::end(ingredients),
                 std::inserter(amounts, begin(amounts)),
                 [](auto const& i) { return i.getAmount(); });

  for(auto i = 0; i < 20; i++) {
    CHECK(amounts.count(i * 10));
  }
}

TEST_CASE("Event Count is Correct") {
  auto timestamp = 12345;

  Roast r{timestamp};

  for(auto i = 0; i < 20; i++) { // NOLINT
    auto timestamp = 20 - i;     // NOLINT
    auto const* eventTypeName = "measurement";
    r.addEvent(Event{eventTypeName, timestamp});
  }

  REQUIRE(r.getEventCount() == 20);
  auto ingredients = RangeGenerator<Event const>(
      [&r](size_t i) -> Event const& { return r.getEvent(i); }, r.getEventCount());
  auto timestamps = std::unordered_set<int>();
  std::transform(std::begin(ingredients), std::end(ingredients),
                 std::inserter(timestamps, begin(timestamps)),
                 [](auto const& i) { return i.getTimestamp(); });

  for(auto i = 1; i < 20; i++) {
    CHECK(timestamps.count(i));
  }
}

TEST_CASE("Ingredient Count is Correct after Removals") {
  auto timestamp = 12345;

  auto beans = std::list<Bean>();
  auto maracatu = Bean("Maracatu");
  Roast r{timestamp};
  for(auto i = 0; i < 19; i++) {
    r.addIngredient(Ingredient{beans.emplace_back("Elimbari" + std::to_string(i)), 10});
  }
  r.addIngredient(Ingredient{maracatu, 5});
  REQUIRE(r.getIngredientsCount() == 20);
  for(auto i = 0; i < 19; i++) {
    r.removeIngredientByBeanName("Elimbari" + std::to_string(i));
  }

  REQUIRE(r.getIngredientsCount() == 1);
  REQUIRE(r.getIngredient(0).getAmount() == 5);
}

TEST_CASE("Event Count is Correct after Removals") {
  auto timestamp = 12345;

  Roast r{timestamp};

  auto const* eventTypeName = "measurement";
  for(auto i = 0; i < 19; i++) {
    auto const timestamp = 12345 + 20 * i;
    r.addEvent(Event{eventTypeName, timestamp});
  }
  r.addEvent(Event{eventTypeName, timestamp + 1});
  REQUIRE(r.getEventCount() == 20);

  for(auto i = 0; i < 19; i++) {
    auto const timestamp = 12345 + 20 * i;
    r.removeEventByTimestamp(timestamp);
  }
  REQUIRE(r.getEventCount() == 1);
  REQUIRE(r.getEvent(0).getTimestamp() == timestamp + 1);
}

TEST_CASE("Breaking ties when getting an event by timestamp") {
  auto timestamp = 12345;

  Roast r{timestamp};

  r.addEvent(Event{"measurement", timestamp + 1});
  r.addEvent(Event{"fill", timestamp});

  REQUIRE(r.getEventByTimestamp(timestamp + 1).getType() == "measurement");
}

TEST_CASE("Removing all the Events until Empty") {
  auto timestamp = 0;

  Roast r{timestamp};

  auto events = std::vector{
      "setting the temperature", "reading the temperature", "filling the machine with beans",
      "browning the beans",      "cracking the beans",      "dropping the beans"};

  for(auto i = 0L; i < events.size(); ++i) {
    r.addEvent(Event{events[i], i, new EventValue{i * 100}});
  }

  for(auto i = 0L; i < events.size(); ++i) {
    r.removeEventByTimestamp(i);
  }

  REQUIRE(r.getEventCount() == 0);
}

TEST_CASE("Removing and Re-adding an Ingredient") {
  auto updatedBean = Bean{"Robusta"};
  auto beans = std::list<Bean>();
  Roast r{10};
  beans.emplace_back("Arabica");
  beans.emplace_back("Robusta");
  beans.emplace_back("Liberica");

  for(auto const& bean : beans) {
    r.addIngredient(Ingredient{bean, 100});
  }

  r.removeIngredientByBeanName("Robusta");
  r.addIngredient(Ingredient{updatedBean, 20});

  REQUIRE(r.getIngredientsCount() == 3);
  for(auto i = 0u; i < r.getIngredientsCount(); i++) {
    if(r.getIngredient(i).getBean().getName() == "Liberica") {
      REQUIRE(r.getIngredient(i).getAmount() == 100);
    } else if(r.getIngredient(i).getBean().getName() == "Robusta") {
      REQUIRE(r.getIngredient(i).getAmount() == 20);
    } else {
      REQUIRE(r.getIngredient(i).getAmount() == 100);
    }
  }
}

TEST_CASE("Updating an Ingredient Amount") {
  auto beans = std::list<Bean>();
  Roast r{10};
  beans.emplace_back("Arabica");
  beans.emplace_back("Robusta");
  beans.emplace_back("Liberica");

  for(auto const& bean : beans) {
    r.addIngredient(Ingredient{bean, 100});
  }

  r.getIngredient(1).setAmount(r.getIngredient(1).getAmount() - 80);

  REQUIRE(r.getIngredientsCount() == 3);
  REQUIRE(r.getIngredient(0).getAmount() == 100);
  REQUIRE(r.getIngredient(1).getAmount() == 20);
  REQUIRE(r.getIngredient(2).getAmount() == 100);
}
