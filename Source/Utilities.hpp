#pragma once
#include <functional>
#include <nlohmann/json.hpp>

#define _STRINGIZE(x) #x
#define STRINGIZE(x) _STRINGIZE(x)

template <typename Result> struct RangeGenerator {
  std::function<Result&(size_t)> generator;
  size_t count;
  struct Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Result;
    using pointer = Result*;
    using reference = Result&;
    std::function<Result&(size_t)> generator;
    size_t i;
    Iterator(std::function<Result&(size_t)> generator, size_t i) : generator(generator), i(i) {}
    bool operator==(Iterator const& other) const { return i == other.i; }
    bool operator!=(Iterator const& other) const { return i != other.i; }
    Iterator& operator++() {
      ++i;
      return *this;
    }
    Result* operator->() { return &generator(i); }
    Result& operator*() { return generator(i); }
  };
  RangeGenerator(std::function<Result&(size_t)> generator, size_t count)
      : generator(generator), count(count) {}
  Iterator begin() const { return Iterator(generator, 0); };
  Iterator end() const { return Iterator(generator, count); };
};
