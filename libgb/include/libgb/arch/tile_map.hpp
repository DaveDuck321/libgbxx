#pragma once

#include <libgb/arch/tile_data.hpp>
#include <libgb/dimensions.hpp>
#include <libgb/std/array.hpp>

namespace libgb {
static constexpr auto tile_map_width = Tiles{32};
static constexpr auto tile_map_height = Tiles{32};

namespace arch {
struct TileMapData {
  using Row = libgb::Array<TileIndex, +tile_map_width>;
  libgb::Array<Row, +tile_map_height> data;

  template <typename Self>
  [[nodiscard]] constexpr auto operator[](this Self &&self, size_t index)
      -> decltype(auto) {
    return self.data[index];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto operator[](this Self &&self, size_t y, size_t x)
      -> decltype(auto) {
    return self.data[y][x];
  }
};

struct TileMapsData {
  libgb::Array<TileMapData, 2> maps;

  template <size_t map_index, typename Self>
  auto fill(this Self &&self, TileIndex index) -> void {
    memset((uint8_t volatile *)self.maps[map_index].data.data(), +index,
           sizeof(self.maps[map_index].data));
  }
};

inline volatile TileMapsData *const tile_maps = (TileMapsData *)0x9800;
} // namespace arch

enum class TileMap : uint8_t {
  map_0 = 0,
  map_1 = 1,
};

template <TileMap map> inline auto fill_tile_mapping(TileIndex tile) -> void {
  memset((uint8_t volatile *)&arch::tile_maps->maps[+map], +tile,
         sizeof(arch::tile_maps->maps[+map]));
}

template <TileMap map>
inline auto set_tile_mapping(Tiles y, Tiles x, TileIndex tile) -> void {
  arch::tile_maps->maps[+map].data[+y][+x] = tile;
}
} // namespace libgb
