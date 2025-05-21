#pragma once

#include <stddef.h>

namespace libgb {
template <auto V> struct Constant {
  using Type = decltype(V);
  static constexpr Type Value = V;

  constexpr Constant() {};
  constexpr operator Type() const { return Value; }
};

template <typename T> struct Identity {
  using Type = T;
};
} // namespace libgb
