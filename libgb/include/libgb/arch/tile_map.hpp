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

  template <size_t map_index, typename Self>
  auto fill(this Self &&self, TileIndex index) -> void {
    memset((uint8_t volatile *)self.maps[map_index].data.data(), +index,
           sizeof(self.maps[map_index].data));
  }
};

inline volatile TileMaps *const tile_maps = (TileMaps *)0x9800;
} // namespace libgb::arch
