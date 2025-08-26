#pragma once

#include <libgb/std/traits.hpp>

namespace libgb {
template <typename T> constexpr auto move(T &&t) -> remove_ref<T> && {
  return static_cast<remove_ref<T> &&>(t);
}

template <typename T> constexpr auto swap(T &lhs, T &rhs) -> void {
  if (&lhs == &rhs) {
    return;
  }

  auto tmp = move(lhs);
  lhs = move(rhs);
  rhs = move(tmp);
}
} // namespace libgb
