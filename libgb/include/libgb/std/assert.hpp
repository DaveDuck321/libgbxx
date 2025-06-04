#pragma once

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

template <typename T> constexpr auto assert_unreachable() -> T {
  assert(false);
  return T{};
}

} // namespace libgb
