#pragma once

#include <libgb/arch/tile_data.hpp>

#include <stdint.h>

namespace libgb::arch {
enum class SpriteDrawingPriority : uint8_t {
  bg_window_all_below = 0,
  bg_window_0_below = 1,
};

struct SpriteAttributes {
  uint8_t reserved : 4;
  uint8_t palette_index : 1;
  bool flip_x : 1;
  bool flip_y : 1;
  SpriteDrawingPriority priority : 1;
};
static_assert(sizeof(SpriteAttributes) == sizeof(uint8_t));

struct Sprite {
  uint8_t pos_y;
  uint8_t pos_x;
  TileIndex index;
  SpriteAttributes attributes;
};
static_assert(sizeof(Sprite) == 4 * sizeof(uint8_t));
} // namespace libgb::arch
