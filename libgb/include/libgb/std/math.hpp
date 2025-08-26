#pragma once

#include <stdint.h>

namespace libgb {
constexpr auto zext(uint8_t value) -> int16_t {
  return static_cast<uint16_t>(value);
}

constexpr auto sext(uint8_t value) -> int16_t {
  return static_cast<int16_t>(static_cast<int8_t>(value));
}

constexpr auto next_power_of_two_mask(uint8_t value) -> int8_t {
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  return value;
}
} // namespace libgb
