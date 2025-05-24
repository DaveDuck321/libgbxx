#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C" {
// c-style signature to catch any calls made by the compiler
auto memcpy(void *dst, void const *src, size_t count) -> void;
}

namespace libgb {
template <typename To, typename From>
constexpr auto bit_cast(From const &from) -> To {
  return __builtin_bit_cast(To, from);
}

inline auto memcpy(uint8_t volatile *dst, uint8_t const *src, size_t count)
    -> void {
  ::memcpy((void *)dst, src, count);
}
} // namespace libgb
