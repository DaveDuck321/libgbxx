#pragma once

#include <stddef.h>

#include "traits.hpp"

namespace libgb {
template <typename T, size_t Size> struct Array {
  T m_data[Size];

  template <typename Self>
  [[nodiscard]] constexpr auto operator[](this Self &&self, size_t index)
      -> decltype(auto) {
    return self.m_data[index];
  }

  template <typename Self> constexpr auto data(this Self &&self) {
    return self.m_data;
  };

  static constexpr auto size() -> size_t { return Size; };

  template <typename Self> constexpr auto begin(this Self &&self) {
    return self.m_data;
  }
  template <typename Self> constexpr auto end(this Self &&self) {
    return self.begin() + self.size();
  }
};

template <typename T> struct Array<T, 0> {
  template <typename Self>
  [[nodiscard]] constexpr auto operator[](this Self &&, size_t)
      -> decltype(auto) {
    __builtin_unreachable();
  }

  template <typename Self> constexpr auto data(this Self &&) {
    return nullptr;
  };

  static constexpr auto size() -> size_t { return 0; };

  // Iterator types are kinda cheaty here, it probably doesn't matter since
  // we're not allowed to deference it.
  template <typename Self> constexpr auto begin(this Self &&) -> T * {
    return nullptr;
  }

  template <typename Self> constexpr auto end(this Self &&) -> T * {
    return nullptr;
  }
};

// Deduction guide
template <typename T, is_same<T>... Ts>
Array(T, Ts...) -> Array<T, sizeof...(Ts) + 1>;
} // namespace libgb

#include "inline_testing.hpp"

INLINE_TEST([] {
  libgb::Array<int, 3> test_array;
  test_array[0] = 6;
  test_array[1] = 7;
  test_array[2] = 8;

  // Is operator[] correct?
  CHECK(test_array[0] == 6);
  CHECK(test_array[1] == 7);
  CHECK(test_array[2] == 8);

  // Is operator[] const correct?
  auto const &array_ref = test_array;
  CHECK(array_ref[0] == 6);
  CHECK(array_ref[1] == 7);
  CHECK(array_ref[2] == 8);

  // Is the interator correct?
  int expected = 6;
  for (auto &member : test_array) {
    CHECK(member == expected++);
    member = 1;
  }
  CHECK(expected == 9);

  // Is the const iterator correct?
  expected = 6;
  for (auto const &member : array_ref) {
    CHECK(member == 1);
  }
  PASS();
})

INLINE_TEST([] {
  // Test the initializer list syntax + the deduction guide
  libgb::Array test_array = {6, 7, 8};
  CHECK(test_array[0] == 6);
  CHECK(test_array[1] == 7);
  CHECK(test_array[2] == 8);
  PASS();
})

INLINE_TEST([] {
  libgb::Array<int, 0> zero_sized_array = {};
  for (auto _ : zero_sized_array) {
    FAIL("Zero length array has item!");
  }

  CHECK(zero_sized_array.size() == 0);
  CHECK(zero_sized_array.data() == nullptr);
  PASS();
})
