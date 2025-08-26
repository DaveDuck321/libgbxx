#pragma once

#include "traits.hpp"

namespace libgb {
constexpr auto assert(bool condition) -> void {
  if (not condition) {
    if consteval {
      throw 0;
    } else {
      asm volatile("trap");
    }
  }
}

template <typename T = void> constexpr auto assert_unreachable() -> T {
  assert(false);
  if constexpr (is_same<T, void>) {
    return;
  } else {
    return T{};
  }
}
} // namespace libgb
