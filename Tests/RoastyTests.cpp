#include "../Source/Roasty.hpp"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
using namespace std;

#include "../Source/Storage/MemoryStorage.hpp"
#include "../Source/Utilities.hpp"

template <typename T> T& peakIntoStorage(Roasty<T>& roasty);
template <> MemoryStorage& peakIntoStorage(Roasty<MemoryStorage>& roasty) {
  return roasty.storage;
};

TEST_CASE("Bean can be CRUD") {
  auto roasty = Roasty<MemoryStorage>();

  REQUIRE(peakIntoStorage(roasty).getBeans().empty());

  SECTION("Adding bean works") {
    roasty.addBean((Bean{"Best Bean"}));
    auto it = std::find_if(peakIntoStorage(roasty).getBeans().begin(),
                           peakIntoStorage(roasty).getBeans().end(),
                           [&](const auto& bean) { return "Best Bean" == bean.getName(); });

    REQUIRE(it != peakIntoStorage(roasty).getBeans().end());
    REQUIRE(it->getName() == "Best Bean");
    peakIntoStorage(roasty).getBeans().clear();
  }

  SECTION("Removing bean works") {
    auto& b = peakIntoStorage(roasty).getBeans().emplace_back("Other bean");
    roasty.deleteBean(b);

    auto it = std::find_if(peakIntoStorage(roasty).getBeans().begin(),
                           peakIntoStorage(roasty).getBeans().end(),
                           [&](const auto& bean) { return b.getName() == bean.getName(); });

    REQUIRE(it == peakIntoStorage(roasty).getBeans().end());
  }
}

TEST_CASE("Roasts can be CRUD") {
  Roasty<MemoryStorage> roasty{};
  auto& storage = peakIntoStorage(roasty);

  SECTION("Adding a roast works") {
    Roast& r = roasty.addRoast(Roast{1000});

    auto it = std::find_if(storage.getRoasts().begin(), storage.getRoasts().end(),
                           [&](const auto& roast) { return 1000 == roast.getTimestamp(); });

    REQUIRE(it != storage.getRoasts().end());
  }

  SECTION("Deleting a roast works") {
    auto& r = storage.getRoasts().emplace_back(50);

    roasty.deleteRoast(50);

    auto it =
        std::find_if(storage.getRoasts().begin(), storage.getRoasts().end(),
                     [&](const auto& roast) { return r.getTimestamp() == roast.getTimestamp(); });

    REQUIRE(it == storage.getRoasts().end());
  }

  SECTION("Adding an event to a roast works") {
    auto& r = storage.getRoasts().emplace_back(50);
    roasty.addEventToRoast(50, {"measurement", 12345});
    roasty.addEventToRoast(50, {"measurement", 123456, new EventValue{200}});

    auto roast = std::find_if(storage.getRoasts().begin(), storage.getRoasts().end(),
                              [](const auto& roast) { return 50 == roast.getTimestamp(); });
    auto matches = 0;
    for(auto i = 0U; i < roast->getEventCount(); i++) {
      if(roast->getEvent(i).getTimestamp() == 123456) {
        REQUIRE(roast->getEvent(i).hasValue());
        REQUIRE(roast->getEvent(i).getValue()->getValue() == 200);
        matches++;
      }
      if(roast->getEvent(i).getTimestamp() == 12345) {
        matches++;
      }
    }
    REQUIRE(matches == 2);
  }

  SECTION("Getting all roasts works") {
    auto& r1 = storage.getRoasts().emplace_back(50);
    auto& r2 = storage.getRoasts().emplace_back(60);

    auto& allRoasts = roasty.allRoasts();

    auto it = std::find_if(allRoasts.begin(), allRoasts.end(),
                           [&](const auto& roast) { return 50 == roast.getTimestamp(); });

    REQUIRE(it != allRoasts.end());

    it = std::find_if(allRoasts.begin(), allRoasts.end(),
                      [&](const auto& roast) { return 60 == roast.getTimestamp(); });

    REQUIRE(it != allRoasts.end());
  }

  SECTION("Getting a roast works") {
    auto& r = storage.getRoasts().emplace_back(65);

    auto& foundRoast = roasty.getRoast(65);

    REQUIRE(foundRoast.getTimestamp() == 65);
  }

  SECTION("Deleting an event from a roast works") {
    {
      Roast r{5678};
      r.addEvent({"measurement", 123459});
      REQUIRE(r.getEventCount() > 0);
      storage.getRoasts().emplace_back(r);
    }

    roasty.removeEventFromRoast(5678, 123459);

    auto& newRoast = roasty.getRoast(5678);

    REQUIRE(newRoast.getEventCount() == 0);
  }

  SECTION("Replacing an event in a roast works") {
    {
      Roast r{5124};
      r.addEvent({"measurement", 123459});
      storage.getRoasts().emplace_back(std::move(r));
    }
    roasty.replaceEventInRoast(5124, 123459, {"measurement", 100000});

    auto& newRoast = roasty.getRoast(5124);
    auto events = RangeGenerator<const Event>(
        [&newRoast](auto i) -> Event const& { return newRoast.getEvent(i); },
        newRoast.getEventCount());

    auto it = std::find_if(events.begin(), events.end(),
                           [&](const auto& event) { return event.getTimestamp() == 100000; });

    REQUIRE(it != events.end());
    REQUIRE(it->getTimestamp() == 100000);
    storage.getRoasts().clear();
  }

  SECTION("Adding a blend to a roast works") {
    auto& javaBean = peakIntoStorage(roasty).getBeans().emplace_back("Java");

    auto& r = storage.getRoasts().emplace_back(5124);

    roasty.addIngredientToRoast(5124, {javaBean, 400});

    auto& newRoast = roasty.getRoast(5124);
    auto bean = RangeGenerator<const Ingredient>(
        [&newRoast](auto i) -> Ingredient const& { return newRoast.getIngredient(i); },
        newRoast.getIngredientsCount());

    auto it = std::find_if(bean.begin(), bean.end(),
                           [](const auto& blend) { return blend.getBean().getName() == "Java"; });

    REQUIRE(newRoast.getIngredientsCount() == 1);
    REQUIRE(it != bean.end());
    REQUIRE(it->getAmount() == 400);
  }

  SECTION("Deleting the only ingredient from a roast works") {
    auto& java = peakIntoStorage(roasty).getBeans().emplace_back("Java");

    {
      Roast r{5123};
      r.addIngredient({java, 500});
      storage.getRoasts().push_back(std::move(r));
    }
    roasty.removeIngredientFromRoast(5123, "Java");

    auto& newRoast = roasty.getRoast(5123);
    auto bean = RangeGenerator<Ingredient const>(
        [&newRoast](auto i) -> Ingredient const& { return newRoast.getIngredient(i); },
        newRoast.getIngredientsCount());

    auto it = std::find_if(bean.begin(), bean.end(),
                           [](const auto& blend) { return blend.getBean().getName() == "Java"; });

    CHECK(it == bean.end());
  }

  SECTION("Deleting one of two ingredients from a roast works") {
    auto& java = peakIntoStorage(roasty).getBeans().emplace_back("Java");
    auto& java2 = peakIntoStorage(roasty).getBeans().emplace_back("Java2");

    {
      Roast r{5123};
      r.addIngredient({java, 500});
      r.addIngredient({java2, 500});

      storage.getRoasts().push_back(std::move(r));
    }
    roasty.removeIngredientFromRoast(5123, "Java");

    auto& newRoast = roasty.getRoast(5123);
    auto bean = RangeGenerator<Ingredient const>(
        [&newRoast](auto i) -> Ingredient const& { return newRoast.getIngredient(i); },
        newRoast.getIngredientsCount());

    auto it = std::find_if(bean.begin(), bean.end(),
                           [](const auto& blend) { return blend.getBean().getName() == "Java"; });

    CHECK(it == bean.end());
  }

  SECTION("Updating a blend from a roast works") {
    {
      Roast r{5123};
      auto& javaBean = storage.getBeans().emplace_back("Java");
      auto b = Ingredient{javaBean, 300};
      r.addIngredient({javaBean, 300});

      storage.getRoasts().push_back(std::move(r));
    }
    roasty.updateIngredient(5123, "Java", 500);

    auto& newRoast = roasty.getRoast(5123);

    REQUIRE(newRoast.getIngredientsCount() == 1);
    REQUIRE(newRoast.getIngredient(0).getAmount() == 500);
  }
}

TEST_CASE("Error handling") {
  auto roasty = Roasty<MemoryStorage>();
  auto& storage = peakIntoStorage(roasty);

  SECTION("Cannot add duplicate event") {
    storage.getRoasts().emplace_back(50);

    roasty.addEventToRoast(50, {"measurement", 12345});

    REQUIRE_THROWS(roasty.addEventToRoast(12, {"measurement", 12345}));
  }

  SECTION("Cannot add event to nonexistent roast") {
    storage.getRoasts().emplace_back(50);

    REQUIRE_THROWS(roasty.addEventToRoast(13, {"measurement", 12345}));
  }

  SECTION("Cannot add duplicate ingredient") {
    storage.getRoasts().emplace_back(50);
    auto& java = peakIntoStorage(roasty).getBeans().emplace_back("Java");

    roasty.addIngredientToRoast(50, {java, 400});

    REQUIRE_THROWS(roasty.addIngredientToRoast(12, {java, 400}));
    peakIntoStorage(roasty).getRoasts().clear();
  }

  SECTION("Cannot add event to nonexistent roast") {
    Roast r{50};
    storage.getRoasts().emplace_back(50);
    auto& java = peakIntoStorage(roasty).getBeans().emplace_back("Java");

    REQUIRE_THROWS(roasty.addIngredientToRoast(300, {java, 400}));
    peakIntoStorage(roasty).getRoasts().clear();
  }
  storage.getRoasts().clear();
}
