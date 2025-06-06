#pragma once

#include <stddef.h>
#include <stdint.h>

namespace libgb {
template <typename To, typename From>
constexpr auto bit_cast(From const &from) -> To {
  return __builtin_bit_cast(To, from);
}

inline auto memcpy(uint8_t volatile *dst, uint8_t const *src, size_t count)
    -> void {
  __builtin_memcpy((void *)dst, src, count);
}

inline auto memset(uint8_t volatile *dst, uint8_t byte, size_t count) -> void {
  __builtin_memset((void *)dst, byte, count);
}
} // namespace libgb
