#pragma once

#include <libgb/std/traits.hpp>

namespace libgb {

template <typename EnumType>
constexpr auto to_underlying(EnumType enum_value) -> underlying_type<EnumType> {
  return static_cast<underlying_type<EnumType>>(enum_value);
}

namespace literals {
template <typename EnumType>
constexpr auto operator+(EnumType enum_value) -> underlying_type<EnumType> {
  return to_underlying(enum_value);
}
} // namespace literals

using namespace literals;
} // namespace libgb
