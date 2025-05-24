#pragma once

#include <stddef.h>

namespace libgb {
using nullptr_t = decltype(nullptr);

template <typename To, typename From>
concept is_convertible = requires(From from) { static_cast<To>(from); };

template <typename T1, typename T2> struct is_same_t {
  static constexpr auto value = false;
};

template <typename T> struct is_same_t<T, T> {
  static constexpr auto value = true;
};

template <typename T1, typename T2>
concept is_same = is_same_t<T1, T2>::value;

template <typename Enum> using underlying_type = __underlying_type(Enum);
} // namespace libgb
