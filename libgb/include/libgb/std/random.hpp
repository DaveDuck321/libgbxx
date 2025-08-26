#pragma once

#include "array.hpp"
#include "math.hpp"
#include "utility.hpp"

#include <stdint.h>

namespace libgb {
auto uniform_random_byte() -> uint8_t;

template <uint8_t min, uint8_t max> auto uniform_in_range() -> uint8_t {
  // Via rejection sampling. Expect 2 samples for adversarial min/max.
  constexpr auto adjusted_max = static_cast<uint8_t>(max - min);
  constexpr auto mask = next_power_of_two_mask(adjusted_max);

  uint8_t sample;
  do {
    sample = static_cast<uint8_t>(mask & uniform_random_byte());
  } while (sample > adjusted_max);

  return sample + min;
}

template <typename T, size_t size>
auto shuffle(libgb::Array<T, size> &array) -> void {
  for (uint8_t i = 0; i < 2 * size; i += 1) {
    uint8_t index = uniform_in_range<0, size - 1>();
    libgb::swap(array[0], array[index]);
  }
}
} // namespace libgb
