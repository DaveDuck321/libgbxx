#pragma once

#include <libgb/std/algorithms.hpp>
#include <libgb/std/array.hpp>

namespace libgb {
template <typename T, size_t Capacity> struct FixedVector {
  // TODO: assert T is trivial
  libgb::Array<T, Capacity> m_data = {};
  size_t m_size = 0;

  template <typename Self>
  [[nodiscard]] constexpr auto operator[](this Self &&self, size_t index)
      -> decltype(auto) {
    return self.m_data[index];
  }

  template <typename Self>
  constexpr auto push_back(this Self &&self, T const &element) -> void {
    self.m_data[self.m_size++] = element;
  }

  template <typename Self> constexpr auto size(this Self &&self) -> size_t {
    return self.m_size;
  }

  template <typename Self> constexpr auto begin(this Self &&self) {
    return self.m_data.begin();
  }
  template <typename Self> constexpr auto end(this Self &&self) {
    return self.m_data.begin() + self.m_size;
  }
};

template <FixedVector vector> consteval auto to_array() {
  return slice<0, vector.m_size>(vector.m_data);
}
} // namespace libgb

#include "inline_testing.hpp"

INLINE_TEST([] {
  libgb::FixedVector<int, 20> vec;
  vec.push_back(2);
  vec.push_back(3);
  vec.push_back(4);
  CHECK(vec[0] == 2);
  CHECK(vec[1] == 3);
  CHECK(vec[2] == 4);

  int count = 0;
  for (auto thing : vec) {
    CHECK(thing == 2 + count);
    count += 1;
  }
  CHECK(count == 3);
  PASS();
});

INLINE_TEST([] {
  constexpr auto vec = [] {
    libgb::FixedVector<int, 20> vec;
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    return vec;
  }();
  libgb::Array<int, 3> target_array = {2, 3, 4};
  CHECK(libgb::to_array<vec>() == target_array);
  PASS();
});
