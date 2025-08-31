#pragma once

#include <libgb/std/limits.hpp>
#include <libgb/std/traits.hpp>

namespace libgb {
template <is_unsigned_integer Underlying> class SaturatingInt {
  Underlying m_underlying;

public:
  constexpr SaturatingInt(Underlying underlying) : m_underlying{underlying} {}

  template <typename Self>
  constexpr auto as_int(this Self &&self) -> Underlying {
    return self.m_underlying;
  }

  template <typename Self>
  constexpr auto operator+=(this Self &&self, Underlying other)
      -> SaturatingInt & {
    auto result = self.m_underlying + other;
    if (result < self.m_underlying) {
      // Unsigned overflow
      self.m_underlying = numeric_limits<Underlying>::max;
    }
    return self;
  }
};
} // namespace libgb
