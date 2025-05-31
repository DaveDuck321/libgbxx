#pragma once

#include <libgb/arch/tile_data.hpp>
#include <libgb/std/array.hpp>

namespace libgb::arch {
struct TileMap {
  using Row = libgb::Array<TileIndex, 32>;
  libgb::Array<Row, 32> data;
};

struct TileMaps {
  libgb::Array<TileMap, 2> maps;
};

inline volatile TileMaps *const tile_maps = (TileMaps *)0x9800;
} // namespace libgb::arch
