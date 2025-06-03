#pragma once

#include <libgb/std/array.hpp>
#include <libgb/std/pair.hpp>
#include <libgb/std/traits.hpp>

namespace libgb {

template <typename SizeType, is_iterable Iterable> struct Enumerator {
  struct End {
    using ContainedIterEnd = decltype(declvalue<Iterable>().end());
    ContainedIterEnd end;
  };

  struct Iterator {
    using ContainedIter = decltype(declvalue<Iterable>().begin());
    using ContainedIterValue = decltype(*declvalue<Iterable>().begin());
    static_assert(is_iterator<ContainedIter>);

    ContainedIter iterator;
    SizeType index = 0;

    constexpr auto operator++() -> Iterator {
      ++iterator;
      index += 1;
      return *this;
    }

    constexpr auto operator*() -> Pair<SizeType, ContainedIterValue> {
      return {index, *iterator};
    }
    constexpr auto operator*() const -> Pair<SizeType, ContainedIterValue> {
      return {index, *iterator};
    }

    constexpr auto operator==(Iterator const &other) const -> bool {
      return iterator == other.iterator;
    }

    constexpr auto operator==(End const &other) const -> bool {
      return other.end == iterator;
    }
  };

  Iterable &other;

  explicit constexpr Enumerator(Iterable &other) : other{other} {}
  constexpr auto begin() -> Iterator { return Iterator{other.begin(), {}}; }
  constexpr auto begin() const -> Iterator {
    return Iterator{other.begin(), {}};
  }

  constexpr auto end() const -> End { return {other.end()}; }
  constexpr auto end() -> End { return {other.end()}; }
};

static_assert(is_iterable<Enumerator<size_t, libgb::Array<int, 6>>>);

template <typename SizeType = size_t, is_iterable Other = void>
constexpr auto enumerate(Other &other) -> Enumerator<SizeType, Other> {
  return Enumerator<SizeType, Other>{other};
}
} // namespace libgb

#include "inline_testing.hpp"

INLINE_TEST([] {
  libgb::Array<int, 3> array = {5, 6, 7};
  for (auto [index, value] : enumerate(array)) {
    CHECK(index + 5 == value);
  }
  PASS();
});

INLINE_TEST([] {
  libgb::Array<int, 3> const array = {5, 6, 7};
  for (auto [index, value] : enumerate(array)) {
    CHECK(index + 5 == value);
  }
  PASS();
});

INLINE_TEST([] {
  libgb::Array<int, 3> array = {5, 6, 7};
  auto enumerator = libgb::enumerate(array);

  for (auto [index, nested_data] : enumerate(enumerator)) {
    auto [index_2, value] = nested_data;
    CHECK(index + 5 == value);
    CHECK(index == index_2);
  }
  PASS();
});
