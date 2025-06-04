#pragma once

#include <libgb/arch/tile.hpp>
#include <libgb/arch/tile_data.hpp>
#include <libgb/std/algorithms.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/assert.hpp>
#include <libgb/std/fixed_vector.hpp>
#include <libgb/std/ranges.hpp>

#include <stdint.h>

namespace libgb {
enum class TileRegistryIndex : size_t {
  invalid_index = ~0U,
};

struct TileRegistry {
  static constexpr auto maximum_tile_count = 1024;
  libgb::FixedVector<arch::Tile, maximum_tile_count> m_all_tiles = {};

  consteval auto find_tile(arch::Tile tile) const -> TileRegistryIndex {
    for (auto const &[index, other_tile] : enumerate(m_all_tiles)) {
      if (other_tile == tile) {
        return TileRegistryIndex{index};
      }
    }
    return TileRegistryIndex::invalid_index;
  }

  consteval auto register_tile(arch::Tile tile) -> TileRegistryIndex {
    if (auto found = find_tile(tile);
        found != TileRegistryIndex::invalid_index) {
      return found;
    }

    auto new_index = m_all_tiles.size();
    m_all_tiles.push_back(tile);
    return TileRegistryIndex{new_index};
  }
};

struct Scene {
  static constexpr size_t tiles_per_region = 128;
  static constexpr size_t sprite_count = 40;

  libgb::Array<TileRegistryIndex, tiles_per_region> m_sprite_tiles = {};
  libgb::Array<TileRegistryIndex, tiles_per_region> m_background_tiles = {};
  libgb::Array<TileRegistryIndex, tiles_per_region> m_shared_tiles = {};

  // TODO: use intra-frame DMA to allow more sprites
  FixedVector<TileIndex, sprite_count> m_sprites = {};

  consteval Scene() {
    for (auto &tile : m_sprite_tiles) {
      tile = TileRegistryIndex::invalid_index;
    }
    for (auto &tile : m_background_tiles) {
      tile = TileRegistryIndex::invalid_index;
    }
    for (auto &tile : m_shared_tiles) {
      tile = TileRegistryIndex::invalid_index;
    }
  }

  consteval auto find_tile_index(
      libgb::Array<TileRegistryIndex, tiles_per_region> const &tile_mapping,
      TileRegistryIndex tile_id) const -> size_t {
    // We allow elements to be deleted (without cascading the correction), we
    // always need to each the entire map.
    // This would be equally correct as a linear search, but we might as well
    // make the happy case (with no hash collision) compile faster.
    static_assert(remove_ref<decltype(tile_mapping)>::size() < 256);
    size_t const start_index = +tile_id % tile_mapping.size();
    size_t search_index = start_index;
    do {
      if (tile_id == tile_mapping[search_index]) {
        return search_index;
      }
      search_index = (search_index + 1) % tile_mapping.size();
    } while (search_index != start_index);
    return tile_mapping.size();
  }

  consteval auto
  insert_tile(libgb::Array<TileRegistryIndex, tiles_per_region> &tile_mapping,
              TileRegistryIndex tile_id) const -> uint8_t {
    // Use the registry index as a placement hint, this allows the tile mapping
    // to remain (relatively) stable between scenes without any expensive
    // placement optimization.
    static_assert(remove_ref<decltype(tile_mapping)>::size() < 256);
    uint8_t const start_index = +tile_id % tile_mapping.size();
    uint8_t search_index = start_index;
    do {
      auto &registry_index = tile_mapping[search_index];
      if (registry_index == TileRegistryIndex::invalid_index) {
        registry_index = tile_id;
        return search_index;
      }

      // Best effort (but non-exhaustive) deduplication of tiles
      assert(registry_index != tile_id);

      search_index = (search_index + 1) % tile_mapping.size();
    } while (search_index != start_index);

    return assert_unreachable<uint8_t>();
  }

  consteval auto insert_double_height_tile(
      libgb::Array<TileRegistryIndex, tiles_per_region> &tile_mapping,
      TileRegistryIndex tile_id_upper, TileRegistryIndex tile_id_lower) const
      -> uint8_t {
    // Use the registry index as a placement hint, this allows the tile mapping
    // to remain (relatively) stable between scenes without any expensive
    // placement optimization.
    static_assert(remove_ref<decltype(tile_mapping)>::size() < 256);
    uint8_t const start_index = +tile_id_upper % tile_mapping.size();
    uint8_t search_index = start_index & 0b10;
    do {
      auto &upper_registry_index = tile_mapping[search_index];
      auto &lower_registry_index = tile_mapping[search_index + 1];
      if (upper_registry_index == TileRegistryIndex::invalid_index &&
          lower_registry_index == TileRegistryIndex::invalid_index) {
        upper_registry_index = tile_id_upper;
        lower_registry_index = tile_id_lower;
        return search_index;
      }

      // Best effort (but non-exhaustive) deduplication of tiles
      assert(upper_registry_index != tile_id_upper);
      assert(lower_registry_index != tile_id_lower);

      search_index = (search_index + 2) % tile_mapping.size();
    } while (search_index != start_index);

    return assert_unreachable<uint8_t>();
  }

  template <typename Registry>
  consteval auto register_background_tile(Registry &registry, arch::Tile tile)
      -> void {
    insert_tile(m_background_tiles, registry.register_tile(tile));
  }

  template <typename Registry>
  consteval auto register_sprite_tile(Registry &registry, arch::Tile tile)
      -> void {
    auto index = insert_tile(m_sprite_tiles, registry.register_tile(tile));
    m_sprites.push_back(TileIndex{(uint8_t)index});
  }

  template <typename Registry>
  consteval auto register_sprite_tiles(Registry &registry,
                                       arch::Tile tile_upper,
                                       arch::Tile tile_lower) -> void {
    auto upper_index = insert_double_height_tile(
        m_sprite_tiles, registry.register_tile(tile_upper),
        registry.register_tile(tile_lower));

    m_sprites.push_back(TileIndex{upper_index});
  }

  template <typename Registry>
  consteval auto background_tile_index(Registry const &registry,
                                       arch::Tile tile) const -> TileIndex {
    auto target_register_index = registry.find_tile(tile);
    for (auto [tile_index, registry_index] :
         enumerate<uint8_t>(m_background_tiles)) {
      if (registry_index == target_register_index) {
        return TileIndex{tile_index};
      }
    }
    throw 0;
  }

  template <typename Registry>
  consteval auto background_tile_address(Registry const &registry,
                                         arch::Tile tile) const -> TileAddress {
    return libgb::tile_address(background_tile_index(registry, tile),
                               libgb::TileAddressingMode::bg_window_signed);
  }

  template <typename Registry>
  consteval auto sprite_tile_index(Registry const &registry,
                                   arch::Tile tile) const -> TileIndex {
    auto target_register_index = registry.find_tile(tile);
    for (auto [tile_index, registry_index] :
         enumerate<uint8_t>(m_sprite_tiles)) {
      if (registry_index == target_register_index) {
        return TileIndex{tile_index};
      }
    }
    throw 0;
  }

  template <typename Registry>
  consteval auto sprite_tile_address(Registry const &registry,
                                     arch::Tile tile) const -> TileAddress {
    return libgb::tile_address(sprite_tile_index(registry, tile),
                               libgb::TileAddressingMode::object);
  }
};

// TODO: shrinkwrap
template <size_t SceneCount> struct AllScenes {
  TileRegistry m_tile_registry;
  Array<Scene, SceneCount> m_scenes;

  template <typename... Scenes>
  explicit consteval AllScenes(TileRegistry tile_registry, Scenes... scenes)
      : m_tile_registry{tile_registry}, m_scenes{scenes...} {}

  consteval auto background_tile_address(size_t scene_id, arch::Tile tile) const
      -> TileAddress {
    return m_scenes[scene_id].background_tile_address(m_tile_registry, tile);
  }

  consteval auto background_tile_index(size_t scene_id, arch::Tile tile) const
      -> TileIndex {
    return m_scenes[scene_id].background_tile_index(m_tile_registry, tile);
  }

  consteval auto sprite_tile_address(size_t scene_id, arch::Tile tile) const
      -> TileAddress {
    return m_scenes[scene_id].sprite_tile_address(m_tile_registry, tile);
  }

  consteval auto sprite_tile_index(size_t scene_id, arch::Tile tile) const
      -> TileIndex {
    return m_scenes[scene_id].sprite_tile_index(m_tile_registry, tile);
  }
};

template <typename... Scenes>
AllScenes(TileRegistry, Scenes... scenes) -> AllScenes<sizeof...(Scenes)>;

template <Scene scene, size_t AllTileCount>
inline auto setup_tiles_for_scene(
    libgb::Array<libgb::arch::Tile, AllTileCount> const &all_tile_data)
    -> void {
  static constexpr auto sprite_mapping = transform(
      scene.m_sprite_tiles, [](size_t index, TileRegistryIndex registry_index) {
        auto address = libgb::tile_address(TileIndex{(uint8_t)index},
                                           TileAddressingMode::object);
        return Pair<TileRegistryIndex, TileAddress>{registry_index, address};
      });

  static constexpr auto shared_mapping = transform(
      scene.m_shared_tiles, [](size_t index, TileRegistryIndex registry_index) {
        auto address =
            libgb::tile_address(TileIndex{(uint8_t)(128U + index)},
                                TileAddressingMode::bg_window_signed);
        return Pair<TileRegistryIndex, TileAddress>{registry_index, address};
      });

  static constexpr auto bg_mapping =
      transform(scene.m_background_tiles, [](size_t index,
                                             TileRegistryIndex registry_index) {
        auto address = libgb::tile_address(
            TileIndex{(uint8_t)index}, TileAddressingMode::bg_window_signed);
        return Pair<TileRegistryIndex, TileAddress>{registry_index, address};
      });

#pragma clang loop unroll(full)
  for (auto [tile, tile_address] : sprite_mapping) {
    if (tile != TileRegistryIndex::invalid_index) {
      set_tile(tile_address, all_tile_data[+tile]);
    }
  }

#pragma clang loop unroll(full)
  for (auto [tile, tile_address] : shared_mapping) {
    if (tile != TileRegistryIndex::invalid_index) {
      set_tile(tile_address, all_tile_data[+tile]);
    }
  }

#pragma clang loop unroll(full)
  for (auto [tile, tile_address] : bg_mapping) {
    if (tile != TileRegistryIndex::invalid_index) {
      set_tile(tile_address, all_tile_data[+tile]);
    }
  }
}

template <AllScenes all_scenes, size_t scene_index>
[[gnu::noinline]] inline auto setup_tiles_for_scene() -> void {
  static constexpr auto scene = all_scenes.m_scenes[scene_index];
  static constexpr auto registry = all_scenes.m_tile_registry;
  static constexpr auto all_tile_data = to_array<registry.m_all_tiles>();
  setup_tiles_for_scene<scene>(all_tile_data);
}
} // namespace libgb
