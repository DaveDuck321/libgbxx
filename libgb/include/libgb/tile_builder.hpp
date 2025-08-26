#pragma once

#include <libgb/arch/tile.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/enum.hpp>
#include <libgb/std/ranges.hpp>

#include <stdint.h>

namespace libgb::tile_builder {
enum Color : uint8_t { id_0, id_1, id_2, id_3 };
using TileColors = libgb::Array<libgb::Array<Color, 8>, 8>;

static constexpr auto C0 = Color::id_0;
static constexpr auto C1 = Color::id_1;
static constexpr auto C2 = Color::id_2;
static constexpr auto C3 = Color::id_3;

consteval auto build_tile(TileColors colors, Color remap_0 = C0,
                          Color remap_1 = C1, Color remap_2 = C2,
                          Color remap_3 = C3) -> arch::Tile {
  libgb::arch::Tile tile = {};
  for (auto [row_index, colors] : enumerate(colors)) {
    uint8_t byte1 = 0;
    uint8_t byte2 = 0;
    for (auto color_before_remap : colors) {
      byte1 <<= 1;
      byte2 <<= 1;

      Color color;
      if (color_before_remap == C0) {
        color = remap_0;
      } else if (color_before_remap == C1) {
        color = remap_1;
      } else if (color_before_remap == C2) {
        color = remap_2;
      } else {
        color = remap_3;
      }

      byte1 |= to_underlying(color) & 1;
      byte2 |= (to_underlying(color) >> 1) & 1;
    }
    tile.data[2 * row_index] = byte1;
    tile.data[2 * row_index + 1] = byte2;
  }
  return tile;
}
} // namespace libgb::tile_builder
