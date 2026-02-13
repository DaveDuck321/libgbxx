#include <libgb/arch/enums.hpp>
#include <libgb/arch/registers.hpp>
#include <libgb/arch/sprite.hpp>
#include <libgb/arch/sprite_map.hpp>
#include <libgb/arch/tile_data.hpp>
#include <libgb/arch/tile_map.hpp>
#include <libgb/dimensions.hpp>
#include <libgb/format.hpp>
#include <libgb/gameloop.hpp>
#include <libgb/input.hpp>
#include <libgb/interrupts.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/assert.hpp>
#include <libgb/std/enum.hpp>
#include <libgb/std/memcpy.hpp>
#include <libgb/std/random.hpp>
#include <libgb/std/ranges.hpp>
#include <libgb/std/saturating_int.hpp>
#include <libgb/tile_allocation.hpp>
#include <libgb/tile_builder.hpp>
#include <libgb/video.hpp>

#include "piece_rotations.hpp"
#include "shared.defs"
#include "stars.hpp"

#include <stdint.h>

using namespace libgb::tile_builder;

namespace {
static constexpr auto hard_drop_force = libgb::Pixels{(uint8_t)-3};
static constexpr auto light_hard_drop_force = libgb::Pixels{(uint8_t)-2};
static constexpr auto side_bump_force = libgb::Pixels{(uint8_t)-3};
static constexpr auto light_side_bump_force = libgb::Pixels{(uint8_t)-2};

static constexpr auto target_scroll_y = libgb::Pixels{(uint8_t)-8};

// For simplicity, the playfield will be indexed from [0, board_width]
// We scroll the screen so that the playing field appears centered
static constexpr auto target_scroll_x = board_screen_offset_x;

static auto scroll_speed_x = libgb::Pixels{0};
static auto scroll_speed_y = libgb::Pixels{0};
static auto scroll_y = target_scroll_y;
static auto scroll_x = target_scroll_x;

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

static constexpr auto piece_tile = build_tile({{
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C2, C2, C2, C1, C1, C1},
    {C0, C1, C2, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto completed_piece_tile = build_tile({{
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C3, C3, C3, C3, C3, C3, C3},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

static constexpr auto hard_drop_tile = build_tile({{
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C0, C0, C0, C0, C0, C1},
    {C0, C1, C0, C0, C0, C0, C0, C1},
    {C0, C1, C0, C0, C0, C0, C0, C1},
    {C0, C1, C0, C0, C0, C0, C0, C1},
    {C0, C1, C0, C0, C0, C0, C0, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C0, C0, C0, C0, C0, C0, C0},
}});

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

static constexpr auto scene_manager = [] {
  libgb::TileRegistry registry;
  libgb::Scene scene;

  scene.register_background_tile(registry, black_tile);
  scene.register_background_tile(registry, piece_tile);
  scene.register_background_tile(registry, completed_piece_tile);
  scene.register_sprite_tile(registry, piece_tile);
  scene.register_sprite_tile(registry, hard_drop_tile);
  scene.register_sprite_tile(registry, completed_piece_tile);

  // Play area boarder
  scene.register_background_tile(registry, left_of_column_tile);
  scene.register_background_tile(registry, right_of_column_tile);
  scene.register_background_tile(registry, top_right_join_tile);
  scene.register_background_tile(registry, top_left_join_tile);
  scene.register_background_tile(registry, bottom_row_tile);

  register_star_sprites(registry, scene);

  return libgb::SceneManager(registry, scene);
}();

auto setup_lcd_controller() -> void {
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();

  libgb::arch::set_lcd_control(libgb::arch::LcdControl{
      .bg_window_enable = true,
      .sprites_enable = true,
      .sprite_size = libgb::arch::SpriteSizeMode::square,
      .bg_tile_map = libgb::arch::TileMapAddressingMode::map_0,
      .tile_data_addressing_mode =
          libgb::arch::TileDataAddressingMode::signed_indexing,
      .window_enable = false,
      .window_tile_map = libgb::arch::TileMapAddressingMode::map_1,
      .lcd_enable = true,
  });

  libgb::arch::set_bg_palette_data(libgb::arch::BgPaletteData{
      .id_0 = libgb::arch::PaletteColor::black,
      .id_1 = libgb::arch::PaletteColor::dark,
      .id_2 = libgb::arch::PaletteColor::light,
      .id_3 = libgb::arch::PaletteColor::white,
  });

  libgb::arch::set_sprite_0_palette_data(libgb::arch::Sprite0PaletteData{
      .padding_0 = 0,
      .id_1 = libgb::arch::PaletteColor::dark,
      .id_2 = libgb::arch::PaletteColor::light,
      .id_3 = libgb::arch::PaletteColor::white,
  });
}

[[gnu::noinline]] auto setup_scene(libgb::ScopedVRAMGuard const &guard)
    -> void {
  libgb::setup_scene_tile_mapping<scene_manager, 0>(guard);
  libgb::fill_tile_mapping<libgb::TileMap::map_0>(
      scene_manager.background_tile_index(0, black_tile));

  // Side boarders
  for (libgb::Tiles row = {}; row < libgb::Tiles{board_height}; ++row) {
    libgb::set_tile_mapping<libgb::TileMap::map_0>(
        row, board_width,
        scene_manager.background_tile_index(0, right_of_column_tile));

    libgb::set_tile_mapping<libgb::TileMap::map_0>(
        row, libgb::tile_map_width - libgb::Tiles{1},
        scene_manager.background_tile_index(0, left_of_column_tile));
  }

  // Bottom boarder
  for (libgb::Tiles column = {}; column < board_width; ++column) {
    libgb::set_tile_mapping<libgb::TileMap::map_0>(
        libgb::Tiles{board_height}, column,
        scene_manager.background_tile_index(0, bottom_row_tile));
  }

  // Fill in the missing pixels along the bottom
  libgb::set_tile_mapping<libgb::TileMap::map_0>(
      libgb::Tiles{board_height}, libgb::tile_map_width - libgb::Tiles{1},
      scene_manager.background_tile_index(0, top_right_join_tile));

  libgb::set_tile_mapping<libgb::TileMap::map_0>(
      libgb::Tiles{board_height}, board_width,
      scene_manager.background_tile_index(0, top_left_join_tile));
}

[[gnu::noinline]] auto setup_audio() -> void {
  libgb::arch::set_audio_master_control_audio_on_off(true);
  libgb::arch::set_sound_panning({
      .channel_1_right = true,
      .channel_2_right = false,
      .channel_3_right = false,
      .channel_4_right = true,
      .channel_1_left = true,
      .channel_2_left = false,
      .channel_3_left = false,
      .channel_4_left = true,
  });
  libgb::arch::set_master_volume_panning({
      .right_volume = 7,
      .vin_right = false,
      .left_volume = 7,
      .vin_left = false,
  });
}

static auto play_hard_drop_sound() -> void {
  libgb::arch::set_channel_1_sweep({
      .step = 7,
      .direction = libgb::arch::SweepDirection::decreases,
      .pace = 1,
      .padding_0 = 0,
  });
  libgb::arch::set_channel_1_length_duty({
      .initial_timer_length = 0,
      .wave_duty = 2,
  });
  libgb::arch::set_channel_1_volume_envelope({
      .pace = 1,
      .envelope_direction = libgb::arch::SweepDirection::decreases,
      .initial_volume = 10,
  });
  libgb::arch::set_channel_1_period_low(libgb::uniform_random_byte());
  libgb::arch::set_channel_1_period_high_control({
      .period = 6,
      .padding_0 = 0,
      .length_enable = false,
      .trigger = true,
  });
}

static auto play_line_clear_sound(uint8_t lines_cleared) -> void {
  libgb::arch::set_channel_1_sweep({
      .step = 7,
      .direction = libgb::arch::SweepDirection::decreases,
      .pace = 1,
      .padding_0 = 0,
  });
  libgb::arch::set_channel_1_length_duty({
      .initial_timer_length = 0,
      .wave_duty = 2,
  });
  libgb::arch::set_channel_1_volume_envelope({
      .pace = 3,
      .envelope_direction = libgb::arch::SweepDirection::decreases,
      .initial_volume = 15,
  });
  libgb::arch::set_channel_1_period_low(200);
  libgb::arch::set_channel_1_period_high_control({
      .period = static_cast<uint8_t>(lines_cleared + 2),
      .padding_0 = 0,
      .length_enable = false,
      .trigger = true,
  });
}

[[maybe_unused]] static auto play_game_over_sound() -> void {
  libgb::arch::set_channel_4_length({
      .initial_timer_length = 0,
      .padding_0 = 0,
  });
  libgb::arch::set_channel_4_volume_envelope({
      .pace = 4,
      .envelope_direction = libgb::arch::SweepDirection::decreases,
      .initial_volume = 15,
  });
  libgb::arch::set_channel_4_frequency_randomness({
      .clock_divider = 1,
      .lfsr_width = libgb::arch::LFSRWidth::long_lfsr,
      .clock_shift = 8,
  });
  libgb::arch::set_channel_4_control({
      .padding_0 = 0,
      .length_enable = false,
      .trigger = true,
  });
}

[[gnu::always_inline]] static auto play_wall_bump_sound() -> void {
  libgb::arch::set_channel_4_length({
      .initial_timer_length = 0,
      .padding_0 = 0,
  });
  libgb::arch::set_channel_4_volume_envelope({
      .pace = 1,
      .envelope_direction = libgb::arch::SweepDirection::decreases,
      .initial_volume = 11,
  });
  libgb::arch::set_channel_4_frequency_randomness({
      .clock_divider = 1,
      .lfsr_width = libgb::arch::LFSRWidth::short_lfsr,
      .clock_shift = 7,
  });
  libgb::arch::set_channel_4_control({
      .padding_0 = 0,
      .length_enable = false,
      .trigger = true,
  });
}

[[gnu::always_inline]] static auto play_soft_drop_sound() -> void {
  libgb::arch::set_channel_4_length({
      .initial_timer_length = 0,
      .padding_0 = 0,
  });
  libgb::arch::set_channel_4_volume_envelope({
      .pace = 1,
      .envelope_direction = libgb::arch::SweepDirection::decreases,
      .initial_volume = 7,
  });
  libgb::arch::set_channel_4_frequency_randomness({
      .clock_divider = 1,
      .lfsr_width = libgb::arch::LFSRWidth::long_lfsr,
      .clock_shift = 2,
  });
  libgb::arch::set_channel_4_control({
      .padding_0 = 0,
      .length_enable = false,
      .trigger = true,
  });
}

struct CurrentGrid {
  static_assert(board_stride > board_width);
  using Row = libgb::Array<libgb::TileIndex,
                           libgb::count_as<libgb::Tiles>(board_stride)>;
  using GridData =
      libgb::Array<Row, libgb::count_as<libgb::Tiles>(board_height)>;
  GridData m_data;
  libgb::Array<uint8_t, libgb::count_as<libgb::Tiles>(board_height)>
      m_tile_count;

  constexpr auto is_occupied_or_out_of_bounds(int8_t y, int8_t x) const
      -> bool {
    if (y < 0) {
      return true;
    }
    if ((uint8_t)x >= libgb::count_as<libgb::Tiles>(board_width)) {
      return true;
    }
    if (y >= libgb::count_as<libgb::Tiles>(board_height)) {
      // Allow free movement above the stage
      return false;
    }
    return !is_empty(libgb::Tiles{static_cast<uint8_t>(y)},
                     libgb::Tiles{static_cast<uint8_t>(x)});
  }

  constexpr auto is_empty(libgb::Tiles y, libgb::Tiles x) const -> bool {
    return m_data[libgb::count_as<libgb::Tiles>(y)]
                 [libgb::count_as<libgb::Tiles>(x)] ==
           scene_manager.background_tile_index(0, black_tile);
  }

  constexpr auto set_full(libgb::Tiles y, libgb::Tiles x) -> void {
    m_data[libgb::count_as<libgb::Tiles>(y)][libgb::count_as<libgb::Tiles>(x)] =
        scene_manager.background_tile_index(0, piece_tile);
    m_tile_count[libgb::count_as<libgb::Tiles>(y)] += 1;
  }

  constexpr auto get_hard_drop_position(uint8_t current_height,
                                        uint8_t column) const -> uint8_t {
    if (current_height >= libgb::count_as<libgb::Tiles>(board_height)) {
      current_height = libgb::count_as<libgb::Tiles>(board_height) - 1;
    }

    for (int8_t row = static_cast<int8_t>(current_height); row >= 0; row -= 1) {
      if (not is_empty(libgb::Tiles{static_cast<uint8_t>(row)},
                       libgb::Tiles{column})) {
        return row + 1;
      }
    }
    return 0;
  }

  constexpr auto fill_row(uint8_t row) -> void {
    libgb::memset(
        (uint8_t *)&m_data[row],
        libgb::to_underlying(
            scene_manager.background_tile_index(0, completed_piece_tile)),
        sizeof(libgb::TileIndex) * libgb::count_as<libgb::Tiles>(board_width));
    m_tile_count[row] = libgb::count_as<libgb::Tiles>(board_width);
  }

  constexpr auto clear_row(uint8_t row) -> void {
    libgb::memset((uint8_t *)&m_data[row],
                  libgb::to_underlying(
                      scene_manager.background_tile_index(0, black_tile)),
                  sizeof(libgb::TileIndex) *
                      libgb::count_as<libgb::Tiles>(board_width));
    m_tile_count[row] = 0;
  }

  constexpr auto mark_rows_as_complete() -> uint8_t {
    uint8_t completed_rows = 0;
    for (auto [y, tile_count] : libgb::enumerate(m_tile_count)) {
      if (tile_count == libgb::count_as<libgb::Tiles>(board_width)) {
        completed_rows += 1;
        libgb::memset((uint8_t *)&m_data[y],
                      libgb::to_underlying(scene_manager.background_tile_index(
                          0, completed_piece_tile)),
                      sizeof(libgb::TileIndex) *
                          libgb::count_as<libgb::Tiles>(board_width));
      }
    }
    return completed_rows;
  }

  constexpr auto delete_cleared_rows() -> void {
    uint8_t old_index = 0;
    uint8_t new_index = 0;
    while (libgb::Tiles{old_index} != board_height) {
      if (libgb::Tiles{m_tile_count[old_index]} == board_width) {
        old_index += 1;
        continue;
      }

      libgb::memcpy((uint8_t *)&m_data[new_index],
                    (uint8_t *)&m_data[old_index],
                    sizeof(libgb::TileIndex) *
                        libgb::count_as<libgb::Tiles>(board_width));
      m_tile_count[new_index] = m_tile_count[old_index];
      old_index += 1;
      new_index += 1;
    }

    // Fill out the remaining empty rows
    while (libgb::Tiles{new_index} != board_height) {
      libgb::memset((uint8_t *)&m_data[new_index],
                    libgb::to_underlying(
                        scene_manager.background_tile_index(0, black_tile)),
                    sizeof(libgb::TileIndex) *
                        libgb::count_as<libgb::Tiles>(board_width));
      m_tile_count[new_index] = 0;
      new_index += 1;
    }
  }
};

constinit static CurrentGrid current_grid = {};

// Defined directly in ASM... The compiler should eventually be good enough to
// generate this tho.
extern "C" void copy_grid_into_vram_map_0(CurrentGrid::GridData *grid);

template <size_t parity> inline auto copy_grid_into_vram() -> void {
#pragma clang loop unroll(full)
  for (uint8_t row = 0; row < libgb::count_as<libgb::Tiles>(board_height);
       row += 1) {
    if (row % 2 != parity) {
      continue;
    }

    auto const tile_y = libgb::Tiles{static_cast<uint8_t>(
        libgb::count_as<libgb::Tiles>(board_height) - row - 1)};
#pragma clang loop unroll(full)
    for (uint8_t column = 0;
         column < libgb::count_as<libgb::Tiles>(board_width); column += 1) {
      auto const tile_x = libgb::Tiles{column};
      libgb::set_tile_mapping<libgb::TileMap::map_0>(
          tile_y, tile_x, current_grid.m_data[row][column]);
    }
  }
}

// Game specific logic
struct FallingPiece {
  static constexpr libgb::Array underlying_piece_sprites = {
      &libgb::inactive_sprite_map[0],
      &libgb::inactive_sprite_map[1],
      &libgb::inactive_sprite_map[2],
      &libgb::inactive_sprite_map[3],
  };

  static constexpr libgb::Array underlying_hard_drop_sprites = {
      &libgb::inactive_sprite_map[4],
      &libgb::inactive_sprite_map[5],
      &libgb::inactive_sprite_map[6],
      &libgb::inactive_sprite_map[7],
  };

  Rotations const *m_piece_rotations;
  Kicks const *m_clockwise_kicks;
  Kicks const *m_counter_clockwise_kicks;

  Coordinate m_position;
  int8_t m_hard_drop_y;
  uint8_t m_rotation;
  uint8_t m_frames_on_ground;

  auto set_underlying_sprite_color_index() -> void {
    auto piece_tile_index = scene_manager.sprite_tile_index(0, piece_tile);
    underlying_piece_sprites[0]->index = piece_tile_index;
    underlying_piece_sprites[1]->index = piece_tile_index;
    underlying_piece_sprites[2]->index = piece_tile_index;
    underlying_piece_sprites[3]->index = piece_tile_index;

    auto hard_drop_tile_index =
        scene_manager.sprite_tile_index(0, hard_drop_tile);
    underlying_hard_drop_sprites[0]->index = hard_drop_tile_index;
    underlying_hard_drop_sprites[1]->index = hard_drop_tile_index;
    underlying_hard_drop_sprites[2]->index = hard_drop_tile_index;
    underlying_hard_drop_sprites[3]->index = hard_drop_tile_index;
  }

  auto render_piece_as_dead() -> void {
    auto piece_tile_index =
        scene_manager.sprite_tile_index(0, completed_piece_tile);
    underlying_piece_sprites[0]->index = piece_tile_index;
    underlying_piece_sprites[1]->index = piece_tile_index;
    underlying_piece_sprites[2]->index = piece_tile_index;
    underlying_piece_sprites[3]->index = piece_tile_index;
    underlying_hard_drop_sprites[0]->index = piece_tile_index;
    underlying_hard_drop_sprites[1]->index = piece_tile_index;
    underlying_hard_drop_sprites[2]->index = piece_tile_index;
    underlying_hard_drop_sprites[3]->index = piece_tile_index;
  }

  constexpr auto is_position_legal(Coordinate position, uint8_t rotation)
      -> bool {
    auto const &current_offsets = (*m_piece_rotations)[rotation];
    for (auto offset : current_offsets) {
      auto tile_coord = offset + position;
      if (current_grid.is_occupied_or_out_of_bounds(tile_coord.y,
                                                    tile_coord.x)) {
        return false;
      }
    }
    return true;
  }

  constexpr auto try_move_right() -> bool {
    if (is_position_legal({(int8_t)(m_position.x + 1), m_position.y},
                          m_rotation)) {
      m_position.x += 1;
      m_frames_on_ground = 0;
      return true;
    }
    return false;
  }

  constexpr auto try_move_left() -> bool {
    if (is_position_legal({(int8_t)(m_position.x - 1), m_position.y},
                          m_rotation)) {
      m_position.x -= 1;
      m_frames_on_ground = 0;
      return true;
    }
    return false;
  }

  constexpr auto try_move_down() -> bool {
    if (is_position_legal({m_position.x, (int8_t)(m_position.y - 1)},
                          m_rotation)) {
      m_position.y -= 1;
      m_frames_on_ground = 0;
      return true;
    }
    return false;
  }

  constexpr auto try_rotate_clockwise() -> bool {
    uint8_t target_rotation = ((uint8_t)(m_rotation + 1)) % 4;
    if (is_position_legal(m_position, target_rotation)) {
      m_rotation = target_rotation;
      m_frames_on_ground = 0;
      return true;
    }

    auto const &kicks = (*m_clockwise_kicks)[m_rotation];
    for (auto kick : kicks) {
      if (is_position_legal(m_position + kick, target_rotation)) {
        m_position = m_position + kick;
        m_rotation = target_rotation;
        m_frames_on_ground = 0;
        return true;
      }
    }
    return false;
  }

  constexpr auto try_rotate_counter_clockwise() -> bool {
    uint8_t target_rotation = ((uint8_t)(m_rotation - 1)) % 4;
    if (is_position_legal(m_position, target_rotation)) {
      m_rotation = target_rotation;
      m_frames_on_ground = 0;
      return true;
    }

    auto const &kicks = (*m_counter_clockwise_kicks)[m_rotation];
    for (auto kick : kicks) {
      if (is_position_legal(m_position + kick, target_rotation)) {
        m_position = m_position + kick;
        m_rotation = target_rotation;
        m_frames_on_ground = 0;
        return true;
      }
    }
    return false;
  }

  auto hard_drop() -> void {
    m_position = {m_position.x, m_hard_drop_y};
    play_hard_drop_sound();
  }

  constexpr auto update_hard_drop_positions() -> void {
    uint8_t minimum_offset = 255;
    auto const &current_offsets = (*m_piece_rotations)[m_rotation];
    for (auto [x_offset, y_offset] : current_offsets) {
      int8_t x = m_position.x + x_offset;
      int8_t y = m_position.y + y_offset;
      uint8_t hard_drop_y = current_grid.get_hard_drop_position(y, x);
      uint8_t offset = y - hard_drop_y;

      if (offset < minimum_offset) {
        minimum_offset = offset;
      }
    }
    m_hard_drop_y = m_position.y - minimum_offset;

    if (minimum_offset == 0) {
      m_frames_on_ground += 1;
    } else {
      m_frames_on_ground = 0;
    }
  }

  auto hide_until_next_update() -> void {
    for (auto *sprite : underlying_piece_sprites) {
      sprite->pos_y = 0;
    }

    for (auto *sprite : underlying_hard_drop_sprites) {
      sprite->pos_y = 0;
    }
  }

  auto copy_position_into_underlying_sprite() -> void {
    auto const &current_offsets = (*m_piece_rotations)[m_rotation];

    size_t index = 0;
    for (auto [x_offset, y_offset] : current_offsets) {
      auto x_tile = libgb::Tiles{(uint8_t)(m_position.x + x_offset)};
      auto y_tile = libgb::Tiles{
          static_cast<uint8_t>(libgb::count_as<libgb::Tiles>(board_height) -
                               (m_position.y + y_offset) - 1)};

      auto *piece_sprite = underlying_piece_sprites[index];
      auto *hard_drop_sprite = underlying_hard_drop_sprites[index];

      piece_sprite->pos_x = libgb::count_px(
          to_px(x_tile) + libgb::sprite_screen_offset.get_width() + scroll_x);
      piece_sprite->pos_y = libgb::count_px(
          to_px(y_tile) - scroll_y + libgb::sprite_screen_offset.get_height());

      auto hard_drop_y_tile = libgb::Tiles{
          static_cast<uint8_t>(libgb::count_as<libgb::Tiles>(board_height) -
                               (m_hard_drop_y + y_offset) - 1)};

      hard_drop_sprite->pos_x = piece_sprite->pos_x;
      hard_drop_sprite->pos_y =
          libgb::count_px(to_px(hard_drop_y_tile) - scroll_y +
                          libgb::sprite_screen_offset.get_height());

      index += 1;
    }
  }

  auto copy_into_current_grid() -> void {
    auto const &current_offsets = (*m_piece_rotations)[m_rotation];

    for (auto [x_offset, y_offset] : current_offsets) {
      auto x = m_position.x + x_offset;
      auto y = m_position.y + y_offset;
      current_grid.set_full(libgb::Tiles{static_cast<uint8_t>(y)},
                            libgb::Tiles{static_cast<uint8_t>(x)});
    }
  }
};

static bool is_game_over = false;
static bool is_level_finished = false;
static bool is_animating_refresh = false;
static FallingPiece falling_piece = {};

static auto generate_falling_piece() -> void {
  auto piece_type = libgb::uniform_in_range<0, 6>();

  switch (piece_type) {
  default:
    __builtin_unreachable();
  case 0: // I
    falling_piece.m_piece_rotations = &i_piece;
    break;
  case 1: // O
    falling_piece.m_piece_rotations = &o_piece;
    break;
  case 2: // T
    falling_piece.m_piece_rotations = &t_piece;
    break;
  case 3: // L
    falling_piece.m_piece_rotations = &l_piece;
    break;
  case 4: // J
    falling_piece.m_piece_rotations = &j_piece;
    break;
  case 5: // S
    falling_piece.m_piece_rotations = &s_piece;
    break;
  case 6: // Z
    falling_piece.m_piece_rotations = &z_piece;
    break;
  }

  switch (piece_type) {
  case 0: // I
    falling_piece.m_clockwise_kicks = &clockwise_I_kicks;
    falling_piece.m_counter_clockwise_kicks = &counter_clockwise_I_kicks;
    break;
  default:
    falling_piece.m_clockwise_kicks = &clockwise_standard_kicks;
    falling_piece.m_counter_clockwise_kicks = &counter_clockwise_standard_kicks;
    break;
  }

  int8_t lower_y = libgb::count_as<libgb::Tiles>(board_height) - 2;
  falling_piece.m_position = {3, lower_y};
  falling_piece.m_rotation = 0;

  // Are we dead?
  if (not falling_piece.is_position_legal(falling_piece.m_position,
                                          falling_piece.m_rotation)) {
    is_game_over = true;
    falling_piece.render_piece_as_dead();
    play_game_over_sound();
    hide_all_stars(false);
  }
}

static uint8_t lines_left_to_clear = 0;
static uint8_t current_level = 0;

auto handle_gameplay_updates() -> void {
  static constexpr libgb::Array<uint8_t, 6> delay_between_shifts = {5, 2, 2,
                                                                    1, 1, 1};

  static bool is_up_pressed = false;
  static bool is_a_pressed = false;
  static bool is_b_pressed = false;
  static bool piece_is_dropped = false;
  static bool has_bumped = false;
  static uint8_t ticks_until_next_shift = 0;
  static uint8_t shift_delay_index = 0;
  static uint8_t frames_since_drop = 0;
  static libgb::Array<uint8_t, 8> frames_between_drop = {40, 10, 5, 2,
                                                         2,  1,  1, 1};
  static constexpr uint8_t frames_before_lock = 40;

  bool is_any_direction_pressed = false;

  if (libgb::is_pressed(libgb::Input::left)) {
    if (ticks_until_next_shift-- == 0) {
      if (!falling_piece.try_move_left()) {
        if (!has_bumped) {
          // Only bump the stage if we're tapping into it or we've had a long
          // run-up
          if (shift_delay_index == 0) {
            scroll_speed_x = -side_bump_force;
            play_wall_bump_sound();
          } else if (shift_delay_index == delay_between_shifts.size() - 1) {
            scroll_speed_x = -light_side_bump_force;
            play_wall_bump_sound();
          }
        }
        has_bumped = true;
      }
      ticks_until_next_shift = delay_between_shifts[shift_delay_index];
      if (shift_delay_index != delay_between_shifts.size() - 1) {
        shift_delay_index += 1;
      }
    }
    is_any_direction_pressed = true;
  }

  if (libgb::is_pressed(libgb::Input::right)) {
    if (ticks_until_next_shift-- == 0) {
      if (!falling_piece.try_move_right()) {
        if (!has_bumped) {
          // Only bump the stage if we're tapping into it or we've had a long
          // run-up
          if (shift_delay_index == 0) {
            scroll_speed_x = side_bump_force;
            play_wall_bump_sound();
          } else if (shift_delay_index == delay_between_shifts.size() - 1) {
            scroll_speed_x = light_side_bump_force;
            play_wall_bump_sound();
          }
        }
        has_bumped = true;
      }
      ticks_until_next_shift = delay_between_shifts[shift_delay_index];
      if (shift_delay_index != delay_between_shifts.size() - 1) {
        shift_delay_index += 1;
      }
    }
    is_any_direction_pressed = true;
  }

  if (libgb::is_pressed(libgb::Input::down)) {
    if (ticks_until_next_shift-- == 0) {
      falling_piece.try_move_down();

      // Down arrow does not accelerate
      ticks_until_next_shift =
          delay_between_shifts[delay_between_shifts.size() - 1];
    }
    is_any_direction_pressed = true;
    frames_since_drop = 0;
  }

  if (libgb::is_pressed(libgb::Input::a)) {
    if (not is_a_pressed) {
      falling_piece.try_rotate_clockwise();
      is_a_pressed = true;
    }
  } else {
    is_a_pressed = false;
  }

  if (libgb::is_pressed(libgb::Input::b)) {
    if (not is_b_pressed) {
      falling_piece.try_rotate_counter_clockwise();
      is_b_pressed = true;
    }
  } else {
    is_b_pressed = false;
  }

  if (not is_any_direction_pressed) {
    ticks_until_next_shift = 0;
    shift_delay_index = 0;
    has_bumped = false;
  }

  falling_piece.update_hard_drop_positions();

  if (libgb::is_pressed(libgb::Input::up)) {
    if (not is_up_pressed) {
      // First frame of up arrow pressed... Hard-drop
      uint8_t drop_amount =
          falling_piece.m_position.y - falling_piece.m_hard_drop_y;
      if (drop_amount >
          libgb::count_as<libgb::Tiles>(board_height - (board_height / 3))) {
        scroll_speed_y = hard_drop_force;
      } else if (drop_amount != 0) {
        scroll_speed_y = light_hard_drop_force;
      }
      falling_piece.hard_drop();
      piece_is_dropped = true;
    }
    is_up_pressed = true;
  } else {
    is_up_pressed = false;
  }

  if (++frames_since_drop > frames_between_drop[current_level]) {
    falling_piece.try_move_down();
    frames_since_drop = 0;
  }

  if (falling_piece.m_frames_on_ground > frames_before_lock) {
    piece_is_dropped = true;
  }

  falling_piece.copy_position_into_underlying_sprite();

  if (piece_is_dropped) {
    if (not is_up_pressed) {
      play_soft_drop_sound();
    }
    falling_piece.copy_into_current_grid();
    lines_left_to_clear = current_grid.mark_rows_as_complete();
    if (lines_left_to_clear == 0) {
      generate_falling_piece();
    } else {
      play_line_clear_sound(lines_left_to_clear);
      if (show_additional_stars<scene_manager>(2 * lines_left_to_clear)) {
        is_level_finished = true;
      }
      falling_piece.hide_until_next_update();
    }
    frames_since_drop = 0;
    piece_is_dropped = false;
  }
}

auto handle_line_clear_animation() -> void {
  static uint8_t line_clear_animation_frame = 0;

  if (line_clear_animation_frame == 4) {
    current_grid.delete_cleared_rows();
    generate_falling_piece();
    falling_piece.update_hard_drop_positions();
    lines_left_to_clear = 0;
    line_clear_animation_frame = 0;
  }

  line_clear_animation_frame += 1;
}

auto handle_level_pass_animation() -> void {
  static uint8_t current_row = 0;
  if (current_row < libgb::count_as<libgb::Tiles>(board_height)) {
    current_grid.fill_row(current_row++);
    if (current_row == libgb::count_as<libgb::Tiles>(board_height)) {
      current_row = 0;
      hide_all_stars(true);
    }
  }
}

auto handle_clear_board_animation() -> void {
  static uint8_t current_row = libgb::count_as<libgb::Tiles>(board_height) - 1;
  if (current_row != 255) {
    current_grid.clear_row(current_row--);
    return;
  }

  is_animating_refresh = false;
  if (is_level_finished) {
    current_level += 1;
    is_level_finished = false;
    current_grid.delete_cleared_rows();
  }
  if (is_game_over) {
    current_level = 0;
    is_game_over = false;
  }
  current_row = libgb::count_as<libgb::Tiles>(board_height) - 1;
  init_stars<scene_manager>();
}
} // namespace

void on_tick() {
  if (target_scroll_y != scroll_y) {
    scroll_speed_y += libgb::Pixels{1};
  } else {
    scroll_speed_y = libgb::Pixels{0};
  }

  if (scroll_x == target_scroll_x) {
    scroll_speed_x = libgb::Pixels{0};
  } else if (scroll_x < target_scroll_x) {
    scroll_speed_x = libgb::Pixels{1};
  } else if (scroll_x > target_scroll_x) {
    scroll_speed_x = -libgb::Pixels{1};
  }

  if (is_animating_refresh) {
    handle_clear_board_animation();
  } else if (is_level_finished) {
    handle_level_pass_animation();
  } else if (is_game_over) {
    falling_piece.copy_position_into_underlying_sprite();
  } else {
    if (lines_left_to_clear == 0) {
      handle_gameplay_updates();
    } else {
      handle_line_clear_animation();
    }
  }

  scroll_y += scroll_speed_y;
  scroll_x += scroll_speed_x;

  bool did_finish_animation =
      animate_stars<scene_manager>(libgb::gameloop::tick_count());
  if (did_finish_animation) {
    if (is_level_finished) {
      play_line_clear_sound(4);
    }
    falling_piece.set_underlying_sprite_color_index();
    is_animating_refresh = true;
  }
}

void on_vblank() {
  copy_grid_into_vram_map_0(&current_grid.m_data);

  libgb::arch::set_background_viewport_x(-count_px(scroll_x));
  libgb::arch::set_background_viewport_y(libgb::count_px(scroll_y));
  libgb::copy_into_active_sprite_map(libgb::inactive_sprite_map);
}

int main() {
  libgb::enable_interrupts();
  setup_lcd_controller();
  libgb::clear_sprite_map(libgb::inactive_sprite_map);
  libgb::copy_into_active_sprite_map(libgb::inactive_sprite_map);
  setup_scene(libgb::ScopedVRAMGuard{});
  setup_audio();
  play_line_clear_sound(4);
  falling_piece.set_underlying_sprite_color_index();
  generate_falling_piece();

  init_stars<scene_manager>();

  return libgb::gameloop::run(on_tick, on_vblank);
}
