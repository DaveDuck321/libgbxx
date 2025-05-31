#pragma once

#include <libgb/arch/sprite.hpp>
#include <libgb/std/array.hpp>

extern "C" void __libgb_do_dma(uint8_t addr_upper);

namespace libgb::arch {
struct [[gnu::packed, gnu::aligned(256)]] SpriteMap {
  libgb::Array<Sprite, 40> data;
};

inline volatile SpriteMap *const active_sprite_map = (SpriteMap *)0xFE00;

[[gnu::always_inline]] inline auto
copy_into_active_sprite_map(SpriteMap const &src) -> void {
  __libgb_do_dma((uint8_t)(((uintptr_t)&src) >> 8U));
}
} // namespace libgb::arch
