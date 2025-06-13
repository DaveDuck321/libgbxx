#include <libgb/arch/enums.hpp>
#include <libgb/arch/registers.hpp>
#include <libgb/arch/sprite.hpp>
#include <libgb/arch/sprite_map.hpp>
#include <libgb/arch/tile_data.hpp>
#include <libgb/arch/tile_map.hpp>
#include <libgb/dimensions.hpp>
#include <libgb/format.hpp>
#include <libgb/interrupts.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/assert.hpp>
#include <libgb/std/enum.hpp>
#include <libgb/std/memcpy.hpp>
#include <libgb/std/random.hpp>
#include <libgb/std/ranges.hpp>
#include <libgb/tile_allocation.hpp>
#include <libgb/tile_builder.hpp>
#include <libgb/video.hpp>

#include "piece_rotations.hpp"

#include <stdint.h>

using namespace libgb::tile_builder;

[[maybe_unused]] static constexpr uint8_t board_width = 10;
[[maybe_unused]] static constexpr uint8_t board_height = 18;
static auto scroll_y = libgb::Pixels{8};

static constexpr auto board_screen_offset =
    (libgb::screen_dims.get_width() - libgb::to_px(libgb::Tiles{board_width})) /
    2;

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
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
    {C0, C1, C1, C1, C1, C1, C1, C1},
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
  scene.register_sprite_tile(registry, piece_tile);
  scene.register_sprite_tile(registry, hard_drop_tile);

  // Play area boarder
  scene.register_background_tile(registry, left_of_column_tile);
  scene.register_background_tile(registry, right_of_column_tile);
  scene.register_background_tile(registry, top_right_join_tile);
  scene.register_background_tile(registry, top_left_join_tile);
  scene.register_background_tile(registry, bottom_row_tile);
  return libgb::SceneManager(registry, scene);
}();

static auto setup_lcd_controller() -> void {
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

[[gnu::noinline]] static auto setup_scene(libgb::ScopedVRAMGuard const &guard)
    -> void {
  libgb::setup_scene_tile_mapping<scene_manager, 0>(guard);
  libgb::fill_tile_mapping<libgb::TileMap::map_0>(
      scene_manager.background_tile_index(0, black_tile));

  // For simplicity, the playfield will be indexed from [0, board_width]
  // We scroll the screen so that the playing field appears centered
  libgb::arch::set_background_viewport_x(-count_px(board_screen_offset));

  // Side boarders
  for (libgb::Tiles row = {}; row < libgb::Tiles{board_height}; ++row) {
    libgb::set_tile_mapping<libgb::TileMap::map_0>(
        row, libgb::Tiles{board_width},
        scene_manager.background_tile_index(0, right_of_column_tile));

    libgb::set_tile_mapping<libgb::TileMap::map_0>(
        row, libgb::tile_map_width - libgb::Tiles{1},
        scene_manager.background_tile_index(0, left_of_column_tile));
  }

  // Bottom boarder
  for (libgb::Tiles column = {}; column < libgb::Tiles{board_width}; ++column) {
    libgb::set_tile_mapping<libgb::TileMap::map_0>(
        libgb::Tiles{board_height}, column,
        scene_manager.background_tile_index(0, bottom_row_tile));
  }

  // Fill in the missing pixels along the bottom
  libgb::set_tile_mapping<libgb::TileMap::map_0>(
      libgb::Tiles{board_height}, libgb::tile_map_width - libgb::Tiles{1},
      scene_manager.background_tile_index(0, top_right_join_tile));

  libgb::set_tile_mapping<libgb::TileMap::map_0>(
      libgb::Tiles{board_height}, libgb::Tiles{board_width},
      scene_manager.background_tile_index(0, top_left_join_tile));
}

struct CurrentGrid {
  using Row = libgb::Array<libgb::TileIndex, board_width + 6>;
  libgb::Array<Row, board_height> data;

  constexpr auto is_occupied_or_out_of_bounds(uint8_t y, uint8_t x) const
      -> bool {
    if (y >= board_height) {
      return true;
    }
    if (x >= board_width) {
      return true;
    }
    return !is_empty(y, x);
  }

  constexpr auto is_empty(uint8_t y, uint8_t x) const -> bool {
    return data[y][x] == scene_manager.background_tile_index(0, black_tile);
  }

  constexpr auto set_full(uint8_t y, uint8_t x) -> void {
    data[y][x] = scene_manager.background_tile_index(0, piece_tile);
  }

  constexpr auto get_hard_drop_position(uint8_t current_height,
                                        uint8_t column) const -> uint8_t {
    for (int8_t row = (int8_t)current_height; row >= 0; row -= 1) {
      if (not is_empty(row, column)) {
        return row + 1;
      }
    }
    return 0;
  }
};

constinit static CurrentGrid current_grid = {};

template <size_t parity> inline auto copy_grid_into_vram() -> void {
#pragma clang loop unroll(full)
  for (uint8_t row = 0; row < board_height; row += 1) {
    if (row % 2 != parity) {
      continue;
    }

    auto const tile_y = libgb::Tiles{(uint8_t)(board_height - row - 1)};
#pragma clang loop unroll(full)
    for (uint8_t column = 0; column < board_width; column += 1) {
      auto const tile_x = libgb::Tiles{column};
      libgb::set_tile_mapping<libgb::TileMap::map_0>(
          tile_y, tile_x, current_grid.data[row][column]);
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
  int8_t m_harddrop_y;
  uint8_t m_rotation;

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

  constexpr auto is_position_legal(Coordinate position, uint8_t rotation)
      -> bool {
    auto const &current_offsets = (*m_piece_rotations)[rotation];
    for (auto offset : current_offsets) {
      auto tile_coord = offset + position;
      if (current_grid.is_occupied_or_out_of_bounds((uint8_t)tile_coord.y,
                                                    (uint8_t)tile_coord.x)) {
        return false;
      }
    }
    return true;
  }

  constexpr auto try_move_right() -> void {
    if (is_position_legal({(int8_t)(m_position.x + 1), m_position.y},
                          m_rotation)) {
      m_position.x += 1;
    }
  }

  constexpr auto try_move_left() -> void {
    if (is_position_legal({(int8_t)(m_position.x - 1), m_position.y},
                          m_rotation)) {
      m_position.x -= 1;
    }
  }

  constexpr auto try_move_down() -> void {
    if (is_position_legal({m_position.x, (int8_t)(m_position.y - 1)},
                          m_rotation)) {
      m_position.y -= 1;
    }
  }

  constexpr auto try_rotate_clockwise() -> void {
    uint8_t target_rotation = ((uint8_t)(m_rotation + 1)) % 4;
    if (is_position_legal(m_position, target_rotation)) {
      m_rotation = target_rotation;
      return;
    }

    auto const &kicks = (*m_clockwise_kicks)[m_rotation];
    for (auto kick : kicks) {
      if (is_position_legal(m_position + kick, target_rotation)) {
        m_position = m_position + kick;
        m_rotation = target_rotation;
        return;
      }
    }
  }

  constexpr auto try_rotate_counter_clockwise() -> void {
    uint8_t target_rotation = ((uint8_t)(m_rotation - 1)) % 4;
    if (is_position_legal(m_position, target_rotation)) {
      m_rotation = target_rotation;
      return;
    }

    auto const &kicks = (*m_counter_clockwise_kicks)[m_rotation];
    for (auto kick : kicks) {
      if (is_position_legal(m_position + kick, target_rotation)) {
        m_position = m_position + kick;
        m_rotation = target_rotation;
        return;
      }
    }
  }

  auto hard_drop() -> void { m_position = {m_position.x, m_harddrop_y}; }

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
    m_harddrop_y = m_position.y - minimum_offset;
  }

  auto copy_position_into_underlying_sprite(libgb::Pixels screen_y_scroll)
      -> void {
    auto const &current_offsets = (*m_piece_rotations)[m_rotation];

    size_t index = 0;
    for (auto [x_offset, y_offset] : current_offsets) {
      auto x_tile = libgb::Tiles{(uint8_t)(m_position.x + x_offset)};
      auto y_tile =
          libgb::Tiles{(uint8_t)(board_height - (m_position.y + y_offset) - 1)};

      auto *piece_sprite = underlying_piece_sprites[index];
      auto *hard_drop_sprite = underlying_hard_drop_sprites[index];

      piece_sprite->pos_x = libgb::count_px(
          to_px(x_tile) + libgb::sprite_screen_offset.get_width() +
          board_screen_offset);
      piece_sprite->pos_y =
          libgb::count_px(to_px(y_tile) - screen_y_scroll +
                          libgb::sprite_screen_offset.get_height());

      auto hard_drop_y_tile =
          libgb::Tiles{(uint8_t)(board_height - (m_harddrop_y + y_offset) - 1)};

      hard_drop_sprite->pos_x = piece_sprite->pos_x;
      hard_drop_sprite->pos_y =
          libgb::count_px(to_px(hard_drop_y_tile) - screen_y_scroll +
                          libgb::sprite_screen_offset.get_height());

      index += 1;
    }
  }

  auto copy_into_current_grid() -> void {
    auto const &current_offsets = (*m_piece_rotations)[m_rotation];

    for (auto [x_offset, y_offset] : current_offsets) {
      auto x = m_position.x + x_offset;
      auto y = m_position.y + y_offset;
      current_grid.set_full(y, x);
    }
  }
};

static FallingPiece falling_piece = {};

static auto generate_falling_piece() -> void {
  falling_piece.set_underlying_sprite_color_index();

  auto piece_type = libgb::uniform_random_byte() % 8;
  while (piece_type == 7) {
    piece_type = libgb::uniform_random_byte() % 8;
  }

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

  int8_t lower_y = board_height - 4;
  falling_piece.m_position = {3, lower_y};
  falling_piece.m_rotation = 0;
}

[[gnu::noinline]] static auto setup_sprites() -> void {
  generate_falling_piece();
}

int main() {
  libgb::enable_interrupts();
  setup_lcd_controller();
  libgb::clear_sprite_map(libgb::inactive_sprite_map);
  libgb::copy_into_active_sprite_map(libgb::inactive_sprite_map);
  setup_scene(libgb::ScopedVRAMGuard{});
  setup_sprites();

  static constexpr libgb::Array<uint8_t, 2> delay_between_shifts = {5, 2};

  static uint8_t frame = 0;

  static bool is_up_pressed = false;
  static bool is_a_pressed = false;
  static bool is_b_pressed = false;
  static bool piece_is_dropped = false;
  static uint8_t ticks_until_next_shift = 0;
  static uint8_t shift_delay_index = 0;

  // Main game loop
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
  while (1) {
    bool is_any_direction_pressed = false;

    libgb::arch::set_joypad_select(libgb::arch::JoypadInputSelect::dpad);
    auto arrows = libgb::arch::get_joypad();

    if (arrows.left_or_b == libgb::arch::JoypadButton::pressed) {
      if (ticks_until_next_shift-- == 0) {
        falling_piece.try_move_left();
        ticks_until_next_shift = delay_between_shifts[shift_delay_index];
        if (shift_delay_index != delay_between_shifts.size() - 1) {
          shift_delay_index += 1;
        }
      }
      is_any_direction_pressed = true;
    }

    if (arrows.right_or_a == libgb::arch::JoypadButton::pressed) {
      if (ticks_until_next_shift-- == 0) {
        falling_piece.try_move_right();
        ticks_until_next_shift = delay_between_shifts[shift_delay_index];
        if (shift_delay_index != delay_between_shifts.size() - 1) {
          shift_delay_index += 1;
        }
      }
      is_any_direction_pressed = true;
    }

    if (arrows.down_or_start == libgb::arch::JoypadButton::pressed) {
      if (ticks_until_next_shift-- == 0) {
        falling_piece.try_move_down();

        // Down arrow does not accelerate
        ticks_until_next_shift =
            delay_between_shifts[delay_between_shifts.size() - 1];
      }
      is_any_direction_pressed = true;
    }

    libgb::arch::set_joypad_select(libgb::arch::JoypadInputSelect::buttons);
    auto buttons = libgb::arch::get_joypad();

    if (buttons.right_or_a == libgb::arch::JoypadButton::pressed) {
      if (not is_a_pressed) {
        falling_piece.try_rotate_clockwise();
        is_a_pressed = true;
      }
    } else {
      is_a_pressed = false;
    }

    if (buttons.left_or_b == libgb::arch::JoypadButton::pressed) {
      if (not is_b_pressed) {
        falling_piece.try_rotate_counter_clockwise();
        is_b_pressed = true;
      }
    } else {
      is_b_pressed = false;
    }

    falling_piece.update_hard_drop_positions();

    if (arrows.up_or_select == libgb::arch::JoypadButton::pressed) {
      if (not is_up_pressed) {
        // First frame of up arrow pressed... Hard-drop
        falling_piece.hard_drop();
        falling_piece.copy_into_current_grid();
        piece_is_dropped = true;
      }
      is_up_pressed = true;
    } else {
      is_up_pressed = false;
    }

    falling_piece.copy_position_into_underlying_sprite(scroll_y);

    if (not is_any_direction_pressed) {
      ticks_until_next_shift = 0;
      shift_delay_index = 0;
    }

    // Commit to VRAM
    frame += 1;
    libgb::wait_for_interrupt<libgb::Interrupt::vblank>();

    // We don't have enough vsync budget to copy the whole grid into vram. Lets
    // do an interlaced copy instead.
    if (frame % 2 == 0) {
      copy_grid_into_vram<0>();
    } else {
      copy_grid_into_vram<1>();
    }
    libgb::arch::set_background_viewport_y(libgb::count_px(scroll_y));

    if (piece_is_dropped) {
      generate_falling_piece();
      piece_is_dropped = false;
    }
    libgb::copy_into_active_sprite_map(libgb::inactive_sprite_map);
  }
}
