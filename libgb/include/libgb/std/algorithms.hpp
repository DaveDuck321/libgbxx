#pragma once

#include <libgb/std/array.hpp>
#include <libgb/std/ranges.hpp>
#include <libgb/std/traits.hpp>
#include <libgb/std/utility.hpp>

#include <stdint.h>

namespace libgb {
struct IsLessThan {
  template <typename T>
  constexpr auto operator()(T const &lhs, T const &rhs) const -> bool {
    return lhs < rhs;
  }
};

template <is_collection Collection, typename Fn = IsLessThan>
consteval auto sort(Collection const &collection, Fn &&is_less_than = {})
    -> Collection {
  Collection result = collection;
  for (unsigned i = 0; i < collection.size(); i += 1) {
    for (unsigned j = 0; j < collection.size() - i - 1; j += 1) {
      if (is_less_than(result[j + 1], result[j])) {
        swap(result[j], result[j + 1]);
      }
    }
  }
  return result;
}

// TODO: it would be nice to have a ranges collect
template <is_collection Collection, typename Fn>
consteval auto transform(Collection const &collection, Fn &&map) {
  Array<decltype(map(0, collection[0])), collection.size()> result;
  for (auto const &[index, item] : enumerate(collection)) {
    result[index] = map(index, item);
  }
  return result;
}

template <typename T, size_t size_lhs, size_t size_rhs>
consteval auto concat(libgb::Array<T, size_lhs> const &lhs,
                      libgb::Array<T, size_rhs> const &rhs) {
  Array<T, size_lhs + size_rhs> result;
  for (auto const &[index, item_lhs] : enumerate(lhs)) {
    result[index] = item_lhs;
  }
  for (auto const &[index, item_rhs] : enumerate(rhs)) {
    result[size_lhs + index] = item_rhs;
  }
  return result;
}

template <size_t Start, size_t End, is_collection Collection>
consteval auto slice(Collection const &collection)
    -> Array<element_type<Collection>, End - Start> {
  Array<element_type<Collection>, End - Start> result;
  for (size_t i = Start; i < End; i += 1) {
    result[i - Start] = collection[i];
  }
  return result;
}
} // namespace libgb

#include "inline_testing.hpp"

INLINE_TEST([] {
  libgb::Array test_array_0 = {1, 2, 3};
  libgb::Array test_array_1 = {2, 1, 3};
  libgb::Array test_array_2 = {3, 2, 1};
  libgb::Array test_array_3 = {3, 1, 2};
  libgb::Array test_array_4 = {2, 3, 1};
  libgb::Array sorted_0 = sort(test_array_0);
  libgb::Array sorted_1 = sort(test_array_1);
  libgb::Array sorted_2 = sort(test_array_2);
  libgb::Array sorted_3 = sort(test_array_3);
  libgb::Array sorted_4 = sort(test_array_4);
  CHECK(test_array_0 == sorted_0);
  CHECK(test_array_0 == sorted_1);
  CHECK(test_array_0 == sorted_2);
  CHECK(test_array_0 == sorted_3);
  CHECK(test_array_0 == sorted_4);
  PASS();
});

INLINE_TEST([] {
  libgb::Array test_array = {1};
  libgb::Array sorted = sort(test_array);
  CHECK(test_array == sorted);
  CHECK(sorted.size() == 1);
  PASS();
});

INLINE_TEST([] {
  libgb::Array<int, 0> test_array = {};
  libgb::Array sorted = sort(test_array);
  CHECK(test_array == sorted);
  CHECK(sorted.size() == 0);
  PASS();
});
