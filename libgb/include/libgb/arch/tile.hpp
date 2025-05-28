#pragma once

#include <libgb/std/array.hpp>

#include <stdint.h>

namespace libgb::arch {
struct Tile {
  libgb::Array<uint8_t, 16> data;

  template <typename Self>
  auto operator=(this Self &&self, auto const &other) -> Self & {
    self.data = other.data;
  }
};
static_assert(8 * sizeof(Tile) == 2 * 8 * 8, "8x8 pixels of 2-bit depth");
} // namespace libgb::arch
