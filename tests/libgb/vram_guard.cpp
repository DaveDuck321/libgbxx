// RUN: $GAMEBOY_EMULATOR_PATH/emulate.out $GBLIB_BUILD_DIR/print.out \
// RUN:   | FileCheck %s -check-prefix=CHECK
#include <libgb/arch/enums.hpp>
#include <libgb/arch/registers.hpp>
#include <libgb/arch/tile_data.hpp>
#include <libgb/arch/tile_map.hpp>
#include <libgb/interrupts.hpp>
#include <libgb/video.hpp>

#include <stdint.h>

static constexpr auto black_tile = [] {
  libgb::arch::Tile tile;
  for (auto &byte : tile.data) {
    byte = 0;
  }
  return tile;
}();

static constexpr auto white_tile = [] {
  libgb::arch::Tile tile;
  for (auto &byte : tile.data) {
    byte = 0xff;
  }
  return tile;
}();

static auto do_large_copy_into_vram() {
  // The emulator will throw an error if there is a contested write into VRAM
  libgb::ScopedVRAMGuard _;

  libgb::arch::registers::set_lcd_control_bg_tile_map(
      libgb::arch::registers::TileMapAddressingMode::map_0);
  libgb::arch::registers::set_lcd_control_tile_data_addressing_mode(
      libgb::arch::registers::TileDataAddressingMode::unsigned_indexing);
  libgb::arch::registers::set_lcd_control_bg_window_enable(1);
  libgb::arch::registers::set_lcd_control_object_enable(0);
  libgb::arch::registers::set_lcd_control_window_enable(0);

  static constexpr auto white_tile_addr = libgb::arch::tile_address(
      libgb::arch::TileIndex{0},
      libgb::arch::TileAddressingMode::bg_window_unsigned);

  for (uint8_t tile_index = 1; tile_index != 0; tile_index += 1) {
    libgb::arch::set_tile(
        libgb::arch::tile_address(
            libgb::arch::TileIndex{1},
            libgb::arch::TileAddressingMode::bg_window_unsigned),
        black_tile);
  }
  libgb::arch::set_tile(white_tile_addr, white_tile);

  libgb::arch::registers::set_bg_palette_data(
      libgb::arch::registers::BgPaletteData{
          .id_0 = libgb::arch::registers::PaletteColor::black,
          .id_1 = libgb::arch::registers::PaletteColor::light,
          .id_2 = libgb::arch::registers::PaletteColor::light,
          .id_3 = libgb::arch::registers::PaletteColor::white,
      });

  for (uint8_t x = 0; x < 32; x += 1) {
    for (uint8_t y = 0; y < 32; y += 1) {
      if ((x + y) % 2 == 0) {
        libgb::arch::tile_maps->maps[0].data[y][x] = libgb::arch::TileIndex{0};
      } else {
        libgb::arch::tile_maps->maps[0].data[y][x] = libgb::arch::TileIndex{1};
      }
    }
  }
}

int main() {
  libgb::enable_interrupts();
  do_large_copy_into_vram();
  libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
  // CHECK: hl=0000
  return 0;
}
