#pragma once

#include <libgb/std/utility.hpp>

#include <stddef.h>

// Convince placement new to be constexpr
constexpr auto operator new(size_t, void *ptr) -> void * { return ptr; }
constexpr auto operator new[](size_t, void *ptr) -> void * { return ptr; }

namespace std {
// This makes me very unhappy... Placement new isn't constexpr EXCEPT when
// called from std::construct_at... We need to wrap our helper in the std
// namespace.
template <class T, class... Args>
constexpr T *construct_at(T *location, Args &&...args) {
  return new (location) T(::libgb::forward<Args>(args)...);
}
} // namespace std

namespace libgb {
template <class T, class... Args>
constexpr T *construct_at(T *location, Args &&...args) {
  return std::construct_at(location, forward<Args>(args)...);
}
} // namespace libgb
