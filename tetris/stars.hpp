#pragma once

#include "libgb/arch/tile_data.hpp"
#include "libgb/std/fixed_dequeue.hpp"
#include "shared.defs"

#include <libgb/dimensions.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/fixed_vector.hpp>
#include <libgb/std/random.hpp>
#include <libgb/tile_allocation.hpp>
#include <libgb/tile_builder.hpp>

#include <stdint.h>

using namespace libgb::tile_builder;

static constexpr auto black_tile = build_tile({{
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto very_little_star_tile_1 = build_tile({{
    {C3, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto little_star_tile_1 = build_tile({{
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C3, C0, C3, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto little_star_tile_2 = build_tile({{
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C3, C3, C3, C0, C0, C0, C0, C0},
    {C0, C3, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto big_star_tile_1 = build_tile({{
    {C3, C0, C3, C0, C3, C0, C0, C0},
    {C0, C3, C3, C3, C0, C0, C0, C0},
    {C3, C3, C0, C3, C3, C0, C0, C0},
    {C0, C3, C3, C3, C0, C0, C0, C0},
    {C3, C0, C3, C0, C3, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto big_star_tile_2 = build_tile({{
    {C3, C0, C3, C0, C3, C0, C0, C0},
    {C0, C3, C3, C3, C0, C0, C0, C0},
    {C3, C3, C3, C3, C3, C0, C0, C0},
    {C0, C3, C3, C3, C0, C0, C0, C0},
    {C3, C0, C3, C0, C3, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto very_big_star_tile_1 = build_tile({{
    {C3, C0, C0, C3, C0, C0, C3, C0},
    {C0, C3, C0, C0, C0, C3, C0, C0},
    {C0, C0, C3, C3, C3, C0, C0, C0},
    {C3, C0, C3, C0, C3, C0, C3, C0},
    {C0, C0, C3, C3, C3, C0, C0, C0},
    {C0, C3, C0, C0, C0, C3, C0, C0},
    {C3, C0, C0, C3, C0, C0, C3, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto very_big_star_tile_2 = build_tile({{
    {C3, C0, C0, C3, C0, C0, C3, C0},
    {C0, C3, C0, C0, C0, C3, C0, C0},
    {C0, C0, C3, C3, C3, C0, C0, C0},
    {C3, C0, C3, C3, C3, C0, C3, C0},
    {C0, C0, C3, C3, C3, C0, C0, C0},
    {C0, C3, C0, C0, C0, C3, C0, C0},
    {C3, C0, C0, C3, C0, C0, C3, C0},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto star_count = 32;
static constexpr auto very_big_star_count = 1;
static constexpr auto big_star_count = 2;
static constexpr auto little_star_count = 12;
static_assert(very_big_star_count + big_star_count + little_star_count <
              star_count);

static consteval auto register_star_sprites(libgb::TileRegistry &registry,
                                            libgb::Scene &scene) -> void {
  scene.register_sprite_tile(registry, black_tile);
  scene.register_sprite_tile(registry, very_little_star_tile_1);
  scene.register_sprite_tile(registry, little_star_tile_1);
  scene.register_sprite_tile(registry, little_star_tile_2);
  scene.register_sprite_tile(registry, big_star_tile_1);
  scene.register_sprite_tile(registry, big_star_tile_2);
  scene.register_sprite_tile(registry, very_big_star_tile_1);
  scene.register_sprite_tile(registry, very_big_star_tile_2);
}

static auto valid_star_tile_positions = libgb::to_array<[]() consteval {
  libgb::FixedVector<libgb::Pair<libgb::Pixels, libgb::Pixels>, 255> result;
  for (auto x_origin = libgb::Pixels{0};
       x_origin < libgb::screen_dims.get_width();
       x_origin += libgb::Pixels{16}) {
    auto x = x_origin + libgb::sprite_screen_offset.get_width();
    for (auto y_origin = libgb::Pixels{0};
         y_origin < libgb::screen_dims.get_height();
         y_origin += libgb::Pixels{16}) {
      auto y = y_origin + libgb::sprite_screen_offset.get_width();
      if (x > (board_screen_offset_x - libgb::tile_dims.width) &&
          x <= board_screen_offset_x + to_px(libgb::Tiles{BOARD_WIDTH})) {
        continue;
      }
      result.push_back({x, y});
    }
  }
  return result;
}()>();

static libgb::Array<uint8_t, star_count> star_show_order;
static uint8_t additional_stars_to_show = 0;
static bool is_hiding_all_stars = false;
static uint8_t star_hide_index = 0;
static uint8_t star_show_index = 0;
static uint8_t delay_frames_for_last_star = 5;

template <auto scene_manager> static auto init_stars() -> void {
  libgb::shuffle(valid_star_tile_positions);

  for (uint8_t sprite_index = 0; sprite_index < star_count; sprite_index += 1) {
    star_show_order[sprite_index] = sprite_index;

    auto y_pos = libgb::uniform_in_range<0, 12>();
    auto x_pos = libgb::uniform_in_range<0, 12>();
    auto origin = valid_star_tile_positions[sprite_index];

    libgb::inactive_sprite_map[8 + sprite_index] = {
        .pos_y = static_cast<uint8_t>(libgb::count_px(origin.second) + y_pos),
        .pos_x = static_cast<uint8_t>(libgb::count_px(origin.first) + x_pos),
        .index = scene_manager.sprite_tile_index(0, black_tile),
        .attributes = {}};
  }
  libgb::shuffle(star_show_order);
}

static auto hide_all_stars() -> void {
  is_hiding_all_stars += true;
  additional_stars_to_show = 0;
}

template <auto scene_manager>
static auto show_additional_stars(uint8_t count) -> void {
  additional_stars_to_show += count;
}

template <auto scene_manager> static auto animate_show_next_star() -> void {
  additional_stars_to_show -= 1;
  auto sprite_index = star_show_order[star_show_index++];
  auto tile = [](uint8_t index) {
    if (index < very_big_star_count) {
      return scene_manager.sprite_tile_index(0, very_big_star_tile_1);
    }
    index -= very_big_star_count;

    if (index < big_star_count) {
      return scene_manager.sprite_tile_index(0, big_star_tile_1);
    }
    index -= big_star_count;

    if (index < little_star_count) {
      return scene_manager.sprite_tile_index(0, little_star_tile_1);
    }

    return scene_manager.sprite_tile_index(0, very_little_star_tile_1);
  }(sprite_index);
  libgb::inactive_sprite_map[8 + sprite_index].index = tile;
}

struct StarAnimation {
  uint8_t frame_to_finish;
  uint8_t sprite_index;
  libgb::TileIndex target_tile;
};

static libgb::FixedDequeue<StarAnimation, 8> star_animation_worklist = {};

template <auto scene_manager>
static auto animate_stars(uint8_t frame_count) -> void {

  if (additional_stars_to_show != 0) {
    if (frame_count % 16 == 0) {
      animate_show_next_star<scene_manager>();
    }
  }

  if (is_hiding_all_stars) [[unlikely]] {
    if (frame_count % 8 == 0) {
      if (star_hide_index != star_show_index) {
        if (star_hide_index + 1 != star_show_index ||
            delay_frames_for_last_star == 0) {
          auto sprite_index = star_show_order[star_hide_index++];
          libgb::inactive_sprite_map[8 + sprite_index].index =
              scene_manager.sprite_tile_index(0, black_tile);
        } else {
          delay_frames_for_last_star -= 1;
        }
      }
    }
  }

  while (not star_animation_worklist.empty()) {
    auto const &next = star_animation_worklist.front();
    if (frame_count == next.frame_to_finish) {
      libgb::inactive_sprite_map[next.sprite_index].index = next.target_tile;
      star_animation_worklist.pop_front();
    } else {
      break;
    }
  }

  if (frame_count % 4 == 0) {
    uint8_t star_to_twinkle_index =
        8 + libgb::uniform_in_range<0, star_count - 1>();
    auto &star_to_twinkle = libgb::inactive_sprite_map[star_to_twinkle_index];

    static constexpr auto very_little_frame_1 =
        scene_manager.sprite_tile_index(0, very_little_star_tile_1);
    static constexpr auto very_little_frame_2 =
        scene_manager.sprite_tile_index(0, black_tile);

    static constexpr auto little_frame_1 =
        scene_manager.sprite_tile_index(0, little_star_tile_1);
    static constexpr auto little_frame_2 =
        scene_manager.sprite_tile_index(0, little_star_tile_2);
    static constexpr auto big_frame_1 =
        scene_manager.sprite_tile_index(0, big_star_tile_1);
    static constexpr auto big_frame_2 =
        scene_manager.sprite_tile_index(0, big_star_tile_2);
    static constexpr auto very_big_frame_1 =
        scene_manager.sprite_tile_index(0, very_big_star_tile_1);
    static constexpr auto very_big_frame_2 =
        scene_manager.sprite_tile_index(0, very_big_star_tile_2);

    switch (star_to_twinkle.index) {
    case very_little_frame_1:
      star_to_twinkle.index = very_little_frame_2;
      star_animation_worklist.push_front({
          .frame_to_finish = static_cast<uint8_t>(frame_count + 1),
          .sprite_index = star_to_twinkle_index,
          .target_tile = very_little_frame_1,
      });
      break;
    case little_frame_1:
      star_to_twinkle.index = little_frame_2;
      star_animation_worklist.push_back({
          .frame_to_finish = static_cast<uint8_t>(frame_count + 4),
          .sprite_index = star_to_twinkle_index,
          .target_tile = little_frame_1,
      });
      break;
    case big_frame_1:
      star_to_twinkle.index = big_frame_2;
      star_animation_worklist.push_back({
          .frame_to_finish = static_cast<uint8_t>(frame_count + 8),
          .sprite_index = star_to_twinkle_index,
          .target_tile = big_frame_1,
      });
      break;
    case very_big_frame_1:
      star_to_twinkle.index = very_big_frame_2;
      star_animation_worklist.push_back({
          .frame_to_finish = static_cast<uint8_t>(frame_count + 8),
          .sprite_index = star_to_twinkle_index,
          .target_tile = very_big_frame_1,
      });
      break;
    }
  }
}
