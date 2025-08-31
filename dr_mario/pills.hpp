#pragma once

#include <libgb/arch/tile_data.hpp>
#include <libgb/std/array.hpp>
#include <libgb/tile_allocation.hpp>
#include <libgb/tile_builder.hpp>

using namespace libgb::tile_builder;

static constexpr auto left_of_column_tile = build_tile({{
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C3},
}});

static constexpr auto right_of_column_tile = build_tile({{
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto bottom_row_tile = build_tile({{
    {C3, C3, C3, C3, C3, C3, C3, C3},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto top_right_join_tile = build_tile({{
    {C0, C0, C0, C0, C0, C0, C0, C3},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto top_left_join_tile = build_tile({{
    {C3, C3, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto dark_pill_left = build_tile({{
    {C0, C0, C0, C1, C1, C1, C1, C1},
    {C0, C0, C1, C1, C2, C2, C2, C1},
    {C0, C1, C1, C2, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C0, C1, C1, C1, C1, C1, C1},
    {C0, C0, C0, C1, C1, C1, C1, C1},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto dark_pill_right = build_tile({{
    {C0, C1, C1, C1, C1, C1, C0, C0},
    {C0, C1, C2, C2, C1, C1, C1, C0},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C0},
    {C0, C1, C1, C1, C1, C1, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto dark_pill_top = build_tile({{
    {C0, C0, C0, C1, C1, C1, C0, C0},
    {C0, C0, C1, C2, C2, C1, C1, C0},
    {C0, C1, C2, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto dark_pill_bottom = build_tile({{
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C2, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C0, C1, C1, C1, C1, C1, C0},
    {C0, C0, C0, C1, C1, C1, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto light_pill_left = build_tile({{
    {C0, C0, C0, C2, C2, C2, C2, C2},
    {C0, C0, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C0, C2, C2, C2, C1, C1, C2},
    {C0, C0, C0, C2, C2, C2, C2, C2},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto light_pill_right = build_tile({{
    {C0, C2, C2, C2, C2, C2, C0, C0},
    {C0, C2, C2, C2, C2, C2, C2, C0},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C1, C1, C1, C2, C2, C0},
    {C0, C2, C2, C2, C2, C2, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto light_pill_top = build_tile({{
    {C0, C0, C0, C2, C2, C2, C0, C0},
    {C0, C0, C2, C2, C2, C2, C2, C0},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto light_pill_bottom = build_tile({{
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C2, C2},
    {C0, C2, C2, C2, C2, C2, C1, C2},
    {C0, C0, C2, C2, C2, C1, C2, C0},
    {C0, C0, C0, C2, C2, C2, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto checkered_pill_left = build_tile({{
    {C0, C0, C0, C2, C1, C2, C1, C2},
    {C0, C0, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C0, C2, C1, C2, C1, C2, C1},
    {C0, C0, C0, C2, C1, C2, C1, C2},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto checkered_pill_right = build_tile({{
    {C0, C2, C1, C2, C1, C2, C0, C0},
    {C0, C1, C2, C1, C2, C1, C2, C0},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C0},
    {C0, C2, C1, C2, C1, C2, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto checkered_pill_top = build_tile({{
    {C0, C0, C0, C2, C1, C2, C0, C0},
    {C0, C0, C2, C1, C2, C1, C2, C0},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto checkered_pill_bottom = build_tile({{
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C1, C2, C1, C2, C1, C2, C1},
    {C0, C2, C1, C2, C1, C2, C1, C2},
    {C0, C0, C2, C1, C2, C1, C2, C0},
    {C0, C0, C0, C2, C1, C2, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

struct PillTile {
  libgb::Array<libgb::arch::Tile, 2> horizontal;
  libgb::Array<libgb::arch::Tile, 2> vertical;

  consteval auto register_tiles(libgb::TileRegistry &registry,
                                libgb::Scene &scene) const -> void {
    scene.register_background_tile(registry, horizontal[0]);
    scene.register_background_tile(registry, horizontal[1]);
    scene.register_background_tile(registry, vertical[0]);
    scene.register_background_tile(registry, vertical[1]);

    scene.register_sprite_tile(registry, horizontal[0]);
    scene.register_sprite_tile(registry, horizontal[1]);
    scene.register_sprite_tile(registry, vertical[0]);
    scene.register_sprite_tile(registry, vertical[1]);
  }
};

static constexpr auto light_pill = PillTile{
    .horizontal = {light_pill_left, light_pill_right},
    .vertical = {light_pill_top, light_pill_bottom},
};

static constexpr auto dark_pill = PillTile{
    .horizontal = {dark_pill_left, dark_pill_right},
    .vertical = {dark_pill_top, dark_pill_bottom},
};

static constexpr auto checked_pill = PillTile{
    .horizontal = {checkered_pill_left, checkered_pill_right},
    .vertical = {checkered_pill_top, checkered_pill_bottom},
};

using Rotations = libgb::Array<libgb::TileIndex, 4>;

template <size_t scene_count>
constexpr auto
get_rotations_background(libgb::SceneManager<scene_count> scene_manager,
                         PillTile const &pill) -> Rotations {
  return {
      scene_manager.background_tile_index(0, pill.horizontal[0]),
      scene_manager.background_tile_index(0, pill.vertical[0]),
      scene_manager.background_tile_index(0, pill.horizontal[1]),
      scene_manager.background_tile_index(0, pill.vertical[1]),
  };
}

template <size_t scene_count>
constexpr auto
get_rotations_sprite(libgb::SceneManager<scene_count> scene_manager,
                     PillTile const &pill) -> Rotations {
  return {
      scene_manager.sprite_tile_index(0, pill.horizontal[0]),
      scene_manager.sprite_tile_index(0, pill.vertical[0]),
      scene_manager.sprite_tile_index(0, pill.horizontal[1]),
      scene_manager.sprite_tile_index(0, pill.vertical[1]),
  };
}
