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
#include <libgb/std/pair.hpp>
#include <libgb/std/random.hpp>
#include <libgb/std/ranges.hpp>
#include <libgb/tile_allocation.hpp>
#include <libgb/tile_builder.hpp>
#include <libgb/video.hpp>

#include "pills.hpp"
#include "shared.defs"

#include <stdint.h>

using namespace libgb::tile_builder;

namespace {
[[maybe_unused]] static constexpr uint8_t board_width = BOARD_WIDTH;
[[maybe_unused]] static constexpr uint8_t board_height = BOARD_HEIGHT;
[[maybe_unused]] static constexpr uint8_t board_stride = BOARD_STRIDE;

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

static constexpr auto board_screen_offset_x =
    (libgb::screen_dims.get_width() - libgb::to_px(libgb::Tiles{board_width})) /
    2;

static constexpr auto target_window_position_y = libgb::Pixels{8};
static constexpr auto target_window_position_x = board_screen_offset_x;

static auto window_position_y = target_window_position_y;
static auto window_position_x = target_window_position_x;

struct Coord {
  int8_t x;
  int8_t y;

  constexpr auto operator+(Coord other) -> Coord {
    return Coord{(int8_t)(x + other.x), (int8_t)(y + other.y)};
  }
};

static constexpr libgb::Array<Coord, 4> wall_kicks = {{
    {0, 0},
    {0, 1},
    {-1, 0},
    {0, -1},
}};

static constexpr libgb::Array<Coord, 4> first_offsets = {
    Coord{0, 0},
    Coord{0, 1},
    Coord{1, 0},
    Coord{0, 0},
};

static constexpr libgb::Array<Coord, 4> second_offsets = {
    Coord{1, 0},
    Coord{0, 0},
    Coord{0, 0},
    Coord{0, 1},
};

static constexpr auto scene_manager = [] {
  libgb::TileRegistry registry;
  libgb::Scene scene;

  scene.register_background_tile(registry, black_tile);

  light_pill.register_tiles(registry, scene);
  dark_pill.register_tiles(registry, scene);
  checked_pill.register_tiles(registry, scene);

  // Play area boarder
  scene.register_background_tile(registry, left_of_column_tile);
  scene.register_background_tile(registry, right_of_column_tile);
  scene.register_background_tile(registry, top_right_join_tile);
  scene.register_background_tile(registry, top_left_join_tile);
  scene.register_background_tile(registry, bottom_row_tile);
  return libgb::SceneManager(registry, scene);
}();

[[maybe_unused]] static constexpr auto light_pill_rotations_background =
    get_rotations_background(scene_manager, light_pill);
[[maybe_unused]] static constexpr auto dark_pill_rotations_background =
    get_rotations_background(scene_manager, dark_pill);
[[maybe_unused]] static constexpr auto checked_pill_rotations_background =
    get_rotations_background(scene_manager, checked_pill);

[[maybe_unused]] static constexpr auto light_pill_rotations_sprite =
    get_rotations_sprite(scene_manager, light_pill);
[[maybe_unused]] static constexpr auto dark_pill_rotations_sprite =
    get_rotations_sprite(scene_manager, dark_pill);
[[maybe_unused]] static constexpr auto checked_pill_rotations_sprite =
    get_rotations_sprite(scene_manager, checked_pill);

auto setup_lcd_controller() -> void {
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();

  libgb::arch::set_lcd_control(libgb::arch::LcdControl{
      .bg_window_enable = true,
      .sprites_enable = true,
      .sprite_size = libgb::arch::SpriteSizeMode::square,
      .bg_tile_map = libgb::arch::TileMapAddressingMode::map_0,
      .tile_data_addressing_mode =
          libgb::arch::TileDataAddressingMode::signed_indexing,
      .window_enable = true,
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
  libgb::fill_tile_mapping<libgb::TileMap::map_1>(
      scene_manager.background_tile_index(0, black_tile));

  // Side boarders
  for (libgb::Tiles row = {}; row < libgb::Tiles{board_height}; ++row) {
    libgb::set_tile_mapping<libgb::TileMap::map_1>(
        row, libgb::Tiles{board_width} + libgb::Tiles{1},
        scene_manager.background_tile_index(0, right_of_column_tile));

    libgb::set_tile_mapping<libgb::TileMap::map_1>(
        row, libgb::Tiles{0},
        scene_manager.background_tile_index(0, left_of_column_tile));
  }

  // Bottom boarder
  for (libgb::Tiles column = {};
       column < libgb::Tiles{board_width} + libgb::Tiles{1}; ++column) {
    libgb::set_tile_mapping<libgb::TileMap::map_1>(
        libgb::Tiles{board_height}, column,
        scene_manager.background_tile_index(0, bottom_row_tile));
  }

  // Fill in the missing pixels along the bottom
  libgb::set_tile_mapping<libgb::TileMap::map_1>(
      libgb::Tiles{board_height}, libgb::Tiles{0},
      scene_manager.background_tile_index(0, top_right_join_tile));

  libgb::set_tile_mapping<libgb::TileMap::map_1>(
      libgb::Tiles{board_height}, libgb::Tiles{board_width} + libgb::Tiles{1},
      scene_manager.background_tile_index(0, top_left_join_tile));
}

enum class PieceColor : uint8_t { none, dark, light, checkered, cleared };

struct CurrentGrid {
  enum class Connection : uint8_t { none, left, right, up, down };

  static_assert(board_stride > board_width);
  using Row = libgb::Array<libgb::TileIndex, board_stride>;
  using GridData = libgb::Array<Row, board_height>;
  using Connections =
      libgb::Array<libgb::Array<Connection, board_stride>, board_height>;
  using Colors =
      libgb::Array<libgb::Array<PieceColor, board_stride>, board_height>;

  [[gnu::aligned(512)]] static inline GridData m_data = {};
  [[gnu::aligned(512)]] static inline Connections m_connections = {};
  [[gnu::aligned(512)]] static inline Colors m_colors = {};

  static constexpr auto is_empty(uint8_t y, uint8_t x) -> bool {
    return m_data[y][x] == scene_manager.background_tile_index(0, black_tile);
  }

  static constexpr auto fill() -> void {
    for (uint8_t y = 5; y < board_height; y += 1) {
      for (uint8_t x = 0; x < board_width; x += 1) {
        m_data[y][x] = scene_manager.background_tile_index(0, light_pill_top);
      }
    }
  }

  static auto handle_falling() -> bool {
    static bool did_fall;
    did_fall = false;
    for (uint8_t y = 1; y < board_height; y += 1) {
      for (uint8_t x = 0; x < board_width; x += 1) {
        if (is_empty(y, x)) {
          continue;
        }

        switch (m_connections[y][x]) {
        default: // None, top, bottom
          if (is_empty(y - 1, x)) {
            did_fall = true;

            m_connections[y - 1][x] = m_connections[y][x];
            m_data[y - 1][x] = m_data[y][x];
            m_colors[y - 1][x] = m_colors[y][x];

            m_data[y][x] = scene_manager.background_tile_index(0, black_tile);
            m_colors[y][x] = PieceColor::none;
          }
          break;
        case Connection::left:
          // We should have already dealt with this as a right connection
          libgb::assert_unreachable();
          break;
        case Connection::right:
          // The entire pill drops in one go
          if (is_empty(y - 1, x) && is_empty(y - 1, x + 1)) {
            did_fall = true;

            m_data[y - 1][x] = m_data[y][x];
            m_data[y - 1][x + 1] = m_data[y][x + 1];
            m_colors[y - 1][x] = m_colors[y][x];
            m_colors[y - 1][x + 1] = m_colors[y][x + 1];

            m_connections[y - 1][x] = m_connections[y][x];
            m_connections[y - 1][x + 1] = m_connections[y][x + 1];

            m_data[y][x] = scene_manager.background_tile_index(0, black_tile);
            m_data[y][x + 1] =
                scene_manager.background_tile_index(0, black_tile);
            m_colors[y][x] = PieceColor::none;
            m_colors[y][x + 1] = PieceColor::none;
          }
          x += 1; // We've dealt with the entire pill
          break;
        }
      }
    }
    return did_fall;
  }

  static auto mark_popped_pieces() -> bool {
    static bool did_pop;
    did_pop = false;

    static libgb::Array<uint8_t, board_width> column_counts;
    static libgb::Array<PieceColor, board_width> column_colors;
    column_counts = {};
    column_colors = {};

    auto unconnect_pill = [&](uint8_t y, uint8_t x) {
      switch (m_connections[y][x]) {
      case Connection::down:
        m_connections[y - 1][x] = Connection::none;
        break;
      case Connection::up:
        m_connections[y + 1][x] = Connection::none;
        break;
      case Connection::left:
        m_connections[y][x - 1] = Connection::none;
        break;
      case Connection::right:
        m_connections[y][x + 1] = Connection::none;
        break;
      default:
        break;
      }
      m_connections[y][x] = Connection::none;
    };

    auto clear_row = [&](uint8_t y, uint8_t end_x, uint8_t size) -> void {
      for (uint8_t x = end_x - size; x < end_x; x += 1) {
        m_data[y][x] =
            scene_manager.background_tile_index(0, black_tile); // TODO
        m_colors[y][x] = PieceColor::cleared;
        unconnect_pill(y, x);
      }
      did_pop = false;
    };

    auto clear_column = [&](uint8_t end_y, uint8_t x, uint8_t size) -> void {
      for (uint8_t y = end_y - size; y < end_y; y += 1) {
        m_data[y][x] =
            scene_manager.background_tile_index(0, black_tile); // TODO
        m_colors[y][x] = PieceColor::cleared;
        unconnect_pill(y, x);
      }
      did_pop = false;
    };

    for (uint8_t y = 0; y < board_height; y += 1) {
      uint8_t row_count = 0;
      PieceColor row_color = PieceColor::none;
      for (uint8_t x = 0; x < board_width; x += 1) {
        auto this_color = m_colors[y][x];

        if (this_color != row_color) {
          if (row_count >= 4) {
            clear_row(y, x, row_count);
          }
          row_count = 1;
          row_color = this_color;
        } else {
          row_count += 1;
        }

        if (this_color != column_colors[x]) {
          if (column_counts[x] >= 4) {
            clear_column(y, x, column_counts[x]);
          }
          column_counts[x] = 1;
          column_colors[x] = this_color;
        } else {
          column_counts[x] += 1;
        }
      }

      if (row_count >= 4) {
        clear_row(y, board_width, row_count);
      }
    }

    for (uint8_t x = 0; x < board_width; x += 1) {
      if (column_counts[x] >= 4) {
        clear_column(board_height, x, column_counts[x]);
      }
    }
    return did_pop;
  }
};

constinit static CurrentGrid current_grid = {};

// Defined directly in ASM... The compiler should eventually be good enough to
// generate this tho.
extern "C" void copy_grid_into_vram_map_1(CurrentGrid::GridData *grid);

struct FallingPiece {
  static constexpr libgb::Array underlying_piece_sprites = {
      &libgb::inactive_sprite_map[0],
      &libgb::inactive_sprite_map[1],
  };

  static inline Rotations const *first_half_rotations_background;
  static inline Rotations const *second_half_rotations_background;

  static inline Rotations const *first_half_rotations_sprite;
  static inline Rotations const *second_half_rotations_sprite;

  static inline uint8_t m_rotation;
  static inline Coord m_position;
  static inline PieceColor m_first_color;
  static inline PieceColor m_second_color;

  static constexpr auto is_position_legal(Coord position, uint8_t rotation)
      -> bool {
    auto first_half_position = position + first_offsets[rotation];
    if (((uint8_t)first_half_position.x) >= board_width ||
        ((uint8_t)first_half_position.y) >= board_height) {
      return false;
    }

    if (not current_grid.is_empty(first_half_position.y,
                                  first_half_position.x)) {
      return false;
    }

    auto second_half_position = position + second_offsets[rotation];
    if (((uint8_t)second_half_position.x) >= board_width ||
        ((uint8_t)second_half_position.y) >= board_height) {
      return false;
    }

    if (not current_grid.is_empty(second_half_position.y,
                                  second_half_position.x)) {
      return false;
    }
    return true;
  }

  static auto try_move_right() -> bool {
    if (is_position_legal({(int8_t)(m_position.x + 1), m_position.y},
                          m_rotation)) {
      m_position.x += 1;
      return true;
    }
    return false;
  }

  static auto try_move_left() -> bool {
    if (is_position_legal({(int8_t)(m_position.x - 1), m_position.y},
                          m_rotation)) {
      m_position.x -= 1;
      return true;
    }
    return false;
  }

  static auto try_move_down() -> bool {
    if (is_position_legal({m_position.x, (int8_t)(m_position.y - 1)},
                          m_rotation)) {
      m_position.y -= 1;
      return true;
    }
    return false;
  }

  static auto try_rotate_clockwise() -> void {
    uint8_t target_rotation = ((uint8_t)(m_rotation + 1)) % 4;
    for (auto kick : wall_kicks) {
      Coord target_coord = m_position + kick;
      if (is_position_legal(target_coord, target_rotation)) {
        m_rotation = target_rotation;
        m_position = target_coord;
        return;
      }
    }
  }

  static auto try_rotate_counter_clockwise() -> void {
    uint8_t target_rotation = ((uint8_t)(m_rotation - 1)) % 4;
    for (auto kick : wall_kicks) {
      Coord target_coord = m_position + kick;
      if (is_position_legal(target_coord, target_rotation)) {
        m_rotation = target_rotation;
        m_position = target_coord;
        return;
      }
    }
  }

  static auto copy_into_current_grid() -> void {
    auto first_position = m_position + first_offsets[m_rotation];
    auto second_position = m_position + second_offsets[m_rotation];
    current_grid.m_data[first_position.y][first_position.x] =
        (*first_half_rotations_background)[m_rotation];
    current_grid.m_data[second_position.y][second_position.x] =
        (*second_half_rotations_background)[(m_rotation + 2) % 4];

    current_grid.m_colors[first_position.y][first_position.x] = m_first_color;
    current_grid.m_colors[second_position.y][second_position.x] =
        m_second_color;

    switch (m_rotation) {
    case 0:
      current_grid.m_connections[first_position.y][first_position.x] =
          CurrentGrid::Connection::right;
      current_grid.m_connections[second_position.y][second_position.x] =
          CurrentGrid::Connection::left;
      break;
    case 1:
      current_grid.m_connections[first_position.y][first_position.x] =
          CurrentGrid::Connection::down;
      current_grid.m_connections[second_position.y][second_position.x] =
          CurrentGrid::Connection::up;
      break;
    case 2:
      current_grid.m_connections[first_position.y][first_position.x] =
          CurrentGrid::Connection::left;
      current_grid.m_connections[second_position.y][second_position.x] =
          CurrentGrid::Connection::right;
      break;
    case 3:
      current_grid.m_connections[first_position.y][first_position.x] =
          CurrentGrid::Connection::up;
      current_grid.m_connections[second_position.y][second_position.x] =
          CurrentGrid::Connection::down;
      break;
    }
  }
};

constinit static FallingPiece falling_piece = {};

auto generate_falling_piece() -> void {
  auto randomly_select_type = [&](Rotations const *&background,
                                  Rotations const *&sprite, PieceColor &color) {
    auto uniform_sample = libgb::uniform_random_byte() % 4;
    while (uniform_sample > 2) {
      uniform_sample = libgb::uniform_random_byte() % 4;
    }
    switch (uniform_sample) {
    default:
      libgb::assert_unreachable();
    case 0:
      background = &dark_pill_rotations_background;
      sprite = &dark_pill_rotations_sprite;
      color = PieceColor::dark;
      break;
    case 1:
      background = &light_pill_rotations_background;
      sprite = &light_pill_rotations_sprite;
      color = PieceColor::light;
      break;
    case 2:
      background = &checked_pill_rotations_background;
      sprite = &checked_pill_rotations_sprite;
      color = PieceColor::checkered;
      break;
    }
  };

  falling_piece.m_rotation = 0;
  falling_piece.m_position.x = 4;
  falling_piece.m_position.y = board_height - 2;
  randomly_select_type(falling_piece.first_half_rotations_background,
                       falling_piece.first_half_rotations_sprite,
                       falling_piece.m_first_color);
  randomly_select_type(falling_piece.second_half_rotations_background,
                       falling_piece.second_half_rotations_sprite,
                       falling_piece.m_second_color);
}

auto hide_falling_piece() -> void {
  falling_piece.underlying_piece_sprites[0]->pos_y = 0;
  falling_piece.underlying_piece_sprites[1]->pos_y = 0;
}

auto render_falling_piece() -> void {
  auto *first_sprite = falling_piece.underlying_piece_sprites[0];
  first_sprite->index =
      (*falling_piece.first_half_rotations_sprite)[falling_piece.m_rotation];

  auto first_tile_x =
      libgb::Tiles{(uint8_t)(falling_piece.m_position.x +
                             first_offsets[falling_piece.m_rotation].x)};
  first_sprite->pos_x = libgb::count_px(
      to_px(first_tile_x) + libgb::sprite_screen_offset.get_width() +
      window_position_x + libgb::to_px(libgb::Tiles{1}));

  auto first_tile_y =
      libgb::Tiles{(uint8_t)(board_height -
                             (falling_piece.m_position.y +
                              first_offsets[falling_piece.m_rotation].y) -
                             1)};
  first_sprite->pos_y = libgb::count_px(
      to_px(first_tile_y) + libgb::sprite_screen_offset.get_height() +
      window_position_y);

  auto *second_sprite = falling_piece.underlying_piece_sprites[1];
  second_sprite->index =
      (*falling_piece
            .second_half_rotations_sprite)[(falling_piece.m_rotation + 2) % 4];

  auto second_tile_x =
      libgb::Tiles{(uint8_t)(falling_piece.m_position.x +
                             second_offsets[falling_piece.m_rotation].x)};
  second_sprite->pos_x = libgb::count_px(
      to_px(second_tile_x) + libgb::sprite_screen_offset.get_width() +
      window_position_x + libgb::to_px(libgb::Tiles{1}));

  auto second_tile_y =
      libgb::Tiles{(uint8_t)(board_height -
                             (falling_piece.m_position.y +
                              second_offsets[falling_piece.m_rotation].y) -
                             1)};
  second_sprite->pos_y = libgb::count_px(
      to_px(second_tile_y) + libgb::sprite_screen_offset.get_height() +
      window_position_y);
}

static bool is_resolving_clear = false;

auto handle_piece_clear() -> void {
  is_resolving_clear = false;

  is_resolving_clear |= current_grid.mark_popped_pieces();

  if (!is_resolving_clear) {
    is_resolving_clear |= current_grid.handle_falling();
  }

  if (!is_resolving_clear) {
    // Final frame of piece clearing
    generate_falling_piece();
  }

  // TODO: run this solver at 60 fps
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
}

auto handle_gameplay_updates() -> void {
  static constexpr libgb::Array<uint8_t, 6> delay_between_shifts = {5, 2, 2,
                                                                    1, 1, 1};

  static bool is_up_pressed = false;
  static bool is_a_pressed = false;
  static bool is_b_pressed = false;
  static bool piece_is_dropped = false;
  static uint8_t ticks_until_next_shift = 0;
  static uint8_t shift_delay_index = 0;

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

  if (not is_any_direction_pressed) {
    ticks_until_next_shift = 0;
    shift_delay_index = 0;
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

  if (arrows.up_or_select == libgb::arch::JoypadButton::pressed) {
    if (not is_up_pressed) {
      piece_is_dropped = true;
    }
    is_up_pressed = true;
  } else {
    is_up_pressed = false;
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

  if (piece_is_dropped) {
    falling_piece.copy_into_current_grid();
    hide_falling_piece();
    piece_is_dropped = false;
    is_resolving_clear = true;
  } else {
    render_falling_piece();
  }
}
} // namespace

int main() {
  libgb::enable_interrupts();
  setup_lcd_controller();
  libgb::clear_sprite_map(libgb::inactive_sprite_map);
  libgb::copy_into_active_sprite_map(libgb::inactive_sprite_map);
  setup_scene(libgb::ScopedVRAMGuard{});

  generate_falling_piece();
  generate_falling_piece();
  generate_falling_piece();

  // Main game loop
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
  while (1) {
    if (is_resolving_clear) {
      handle_piece_clear();
    } else {
      handle_gameplay_updates();
    }

    libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
    // libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
    //  copy_grid_into_vram_map_1((CurrentGrid::GridData*)&current_grid.m_colors);
    copy_grid_into_vram_map_1(&current_grid.m_data);

    libgb::arch::set_window_position_y(libgb::to_underlying(window_position_y));
    libgb::arch::set_window_position_x_plus_7(
        libgb::to_underlying(window_position_x) + 7);

    libgb::copy_into_active_sprite_map(libgb::inactive_sprite_map);
  }
}
