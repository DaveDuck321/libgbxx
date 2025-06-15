#pragma once

#include <libgb/std/enum.hpp>
#include <libgb/std/traits.hpp>

#include <stdint.h>

namespace libgb {
enum class Pixels : uint8_t {};
enum class Tiles : uint8_t {};

constexpr auto count_px(Pixels pixels) -> uint8_t {
  return to_underlying(pixels);
}

constexpr auto to_px(Pixels measure) -> Pixels { return measure; }
constexpr auto to_px(Tiles measure) -> Pixels {
  return Pixels{static_cast<uint8_t>(8 * +measure)};
}

template <typename T>
concept is_measure_unit = requires(T t) { to_px(t); };

template <is_measure_unit Unit>
constexpr auto operator+(Unit lhs, Unit rhs) -> Unit {
  return static_cast<Unit>(to_underlying(lhs) + to_underlying(rhs));
}

template <is_measure_unit Unit> constexpr auto operator-(Unit value) -> Unit {
  return static_cast<Unit>(-to_underlying(value));
}

template <is_measure_unit Unit>
constexpr auto operator++(Unit &value) -> Unit & {
  value = static_cast<Unit>(to_underlying(value) + 1);
  return value;
}

template <is_measure_unit Unit>
constexpr auto operator+=(Unit &value, Unit const &other) -> Unit & {
  value = value + other;
  return value;
}

template <is_measure_unit Unit>
constexpr auto operator-=(Unit &value, Unit const &other) -> Unit & {
  value = value - other;
  return value;
}

template <is_measure_unit Unit>
constexpr auto operator-(Unit lhs, Unit rhs) -> Unit {
  return static_cast<Unit>(to_underlying(lhs) - to_underlying(rhs));
}

template <typename ScalarType, is_measure_unit Unit>
constexpr auto operator*(ScalarType lhs, Unit rhs) -> Unit {
  return static_cast<Unit>(lhs * to_underlying(rhs));
}

template <is_measure_unit Unit, typename ScalarType>
consteval auto operator/(Unit lhs, ScalarType rhs) -> Unit {
  return static_cast<Unit>(to_underlying(lhs) / rhs);
}

template <is_measure_unit Unit>
consteval auto operator/(Unit lhs, Unit rhs) -> underlying_type<Unit> {
  return to_underlying(lhs) / to_underlying(rhs);
}

struct Dimension {
  Pixels width;
  Pixels height;

  template <is_measure_unit Unit = Pixels>
  consteval auto get_width() const -> Unit {
    return (width / to_px(Unit{1})) * Unit{1};
  }

  template <is_measure_unit Unit = Pixels>
  consteval auto get_height() const -> Unit {
    return (height / to_px(Unit{1})) * Unit{1};
  }
};

static constexpr auto screen_dims = Dimension{
    .width = Pixels{160},
    .height = Pixels{144},
};

static constexpr auto tile_dims = Dimension{
    .width = Pixels{8},
    .height = Pixels{8},
};

static constexpr auto sprite_screen_offset = Dimension{
    .width = Pixels{8},
    .height = Pixels{16},
};
} // namespace libgb
