#pragma once

#include <libgb/arch/sprite.hpp>
#include <libgb/std/array.hpp>

extern "C" void __libgb_do_dma(uint8_t addr_upper);

namespace libgb {
namespace arch {
struct [[gnu::packed, gnu::aligned(256)]] SpriteMap {
  libgb::Array<Sprite, 40> data;

  template <typename Self>
  constexpr auto operator[](this Self &&self, uint8_t index) -> decltype(auto) {
    return self.data[index];
  }
};

inline volatile SpriteMap *const active_sprite_map = (SpriteMap *)0xFE00;
} // namespace arch

inline arch::SpriteMap inactive_sprite_map = {};

[[gnu::always_inline]] inline auto
copy_into_active_sprite_map(arch::SpriteMap const &src) -> void {
  __libgb_do_dma((uint8_t)(((uintptr_t)&src) >> 8U));
}

[[gnu::always_inline]] inline auto clear_sprite_map(arch::SpriteMap &dst)
    -> void {
  // Hides all sprites and resets attributes to default
  memset((uint8_t *)&dst.data, 0, sizeof(dst.data));
}
} // namespace libgb
