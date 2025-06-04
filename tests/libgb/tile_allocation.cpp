// RUN: $GB_TOOLCHAIN/llvm-objdump $GBLIB_BUILD_DIR/tile_allocation.out -d \
// RUN:   | FileCheck %s -check-prefix=CHECK

#include <libgb/interrupts.hpp>
#include <libgb/tile_allocation.hpp>
#include <libgb/video.hpp>

#include <stdint.h>

static constexpr auto black_tile = [] {
  libgb::arch::Tile tile;
  for (auto &byte : tile.data) {
    byte = 0;
  }
  return tile;
}();

static constexpr auto gray_tile = [] {
  size_t index = 0;
  libgb::arch::Tile tile;
  for (auto &byte : tile.data) {
    if (index++ % 2 == 0) {
      byte = 0x00;
    } else {
      byte = 0xff;
    }
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

static constexpr auto stripe_tile_1 = [] {
  libgb::arch::Tile tile;
  for (auto &byte : tile.data) {
    byte = 0xee;
  }
  return tile;
}();

static constexpr auto stripe_tile_2 = [] {
  libgb::arch::Tile tile;
  for (auto &byte : tile.data) {
    byte = 0x0e;
  }
  return tile;
}();

static constexpr auto all_scenes = [] {
  libgb::TileRegistry registry;
  libgb::Scene scene1;
  scene1.register_background_tile(registry, black_tile);
  scene1.register_background_tile(registry, white_tile);
  scene1.register_sprite_tile(registry, gray_tile);

  libgb::Scene scene2;
  scene2.register_sprite_tile(registry, black_tile);
  scene2.register_sprite_tiles(registry, stripe_tile_1, stripe_tile_2);
  scene2.register_background_tile(registry, white_tile);
  return libgb::AllScenes(registry, scene1, scene2);
}();

static auto setup_vram() -> void {
  asm volatile("debugtrap" ::: "memory");
  libgb::ScopedVRAMGuard _;

  libgb::setup_tiles_for_scene<all_scenes, 0>();
  // TODO: the compiler should combine these memcpys
  // memcpy to sprite tile data
  // CHECK: ld hl, $8020
  // CHECK: ld bc, $4020
  // CHECK: ld de, $0010
  // CHECK: call

  // memcpy to bg tile data
  // CHECK: ld hl, $9000
  // CHECK: ld bc, $4000
  // CHECK: ld de, $0010
  // CHECK: call
  // CHECK: ld hl, $9010
  // CHECK: ld bc, $4010
  // CHECK: ld de, $0010
  // CHECK: call

  libgb::setup_tiles_for_scene<all_scenes, 1>();
  // memcpy to sprite tile data
  // CHECK: ld hl, $8000
  // CHECK: ld bc, $4050
  // CHECK: ld de, $0010
  // CHECK: call
  // CHECK: ld hl, $8020
  // CHECK: ld bc, $4080
  // CHECK: ld de, $0010
  // CHECK: call
  // CHECK: ld hl, $8030
  // CHECK: ld bc, $4090
  // CHECK: ld de, $0010
  // CHECK: call

  // memcpy to bg tile data
  // CHECK: ld hl, $9010
  // CHECK: ld bc, $4060
  // CHECK: ld de, $0010
  // CHECK: call

  asm volatile("debugtrap" ::: "memory");
}

int main() {
  libgb::enable_interrupts();
  setup_vram();
}
