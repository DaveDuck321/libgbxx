#pragma once

#include <libgb/std/traits.hpp>

#include <stdint.h>

namespace libgb {
template <typename T> struct numeric_limits {};

template <is_signed_integer T> struct numeric_limits<T> {
  static constexpr T min = ~T{};
  static constexpr T max = ~(min + 1);
};

template <is_unsigned_integer T> struct numeric_limits<T> {
  static constexpr T min = 0;
  static constexpr T max = ~T{};
};
}; // namespace libgb
