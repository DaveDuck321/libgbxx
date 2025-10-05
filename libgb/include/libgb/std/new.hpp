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
constexpr auto construct_at(T *location, Args &&...args) -> T * {
  return new (location) T(::libgb::forward<Args>(args)...);
}

template <class T, class... Args>
constexpr auto destroy_at(T *location) -> void {
  location->~T();
}
} // namespace std

namespace libgb {
template <class T, class... Args>
constexpr auto construct_at(T *location, Args &&...args) -> T * {
  return std::construct_at(location, forward<Args>(args)...);
}

template <class T> constexpr auto destroy_at(T *location) -> void {
  return std::destroy_at(location);
}
} // namespace libgb
