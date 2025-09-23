#include <libgb/arch/registers.hpp>
#include <libgb/input.hpp>
#include <libgb/std/bit.hpp>

#include <stdint.h>

namespace libgb::impl {
Input input_state;
}

auto libgb::read_inputs() -> void {
  // read_inputs() has exclusive ownership of the joypad registers
  // Assume that the dpad is currently selected.
  auto result = libgb::bitcast<uint8_t>(libgb::arch::get_joypad());
  libgb::arch::set_joypad_select(libgb::arch::JoypadInputSelect::buttons);

  result <<= 4;

  // I've got no idea what the timing here needs to be here.
  // Best to just be pessimistic and do what the interent recommends.
  // Use repeated reads to `get_joypad` as an expensive nop.
  // If we wanted to shave cycles, we could count the cycles from the `result`
  // shift+mask.
#pragma unroll
  for (uint8_t i = 0; i < 5; i += 1) {
    (void)libgb::arch::get_joypad();
  }

  auto arrows = libgb::bitcast<uint8_t>(libgb::arch::get_joypad());
  result |= (arrows & 0x0fu);

  libgb::impl::input_state = static_cast<Input>(result);

  libgb::arch::set_joypad_select(libgb::arch::JoypadInputSelect::dpad);
}
