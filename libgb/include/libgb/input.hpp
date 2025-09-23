#pragma once

#include <stdint.h>

#include <libgb/std/enum.hpp>

namespace libgb {
enum class Input : uint8_t {
  a = 1U << 0,
  b = 1U << 1,
  select = 1U << 2,
  start = 1U << 3,

  right = 1U << 4,
  left = 1U << 5,
  up = 1U << 6,
  down = 1U << 7,
};

namespace impl {
extern Input input_state;
}

/**
 * Read the current input state from the hardware.
 * Requires exclusive ownership of the joypad register.
 */
auto read_inputs() -> void;

[[gnu::always_inline]] inline auto is_pressed(Input input) -> bool {
  return (+impl::input_state & +input) == 0;
}

} // namespace libgb
