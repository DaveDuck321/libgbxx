#pragma once

#include <libgb/arch/registers.hpp>
#include <libgb/std/constant.hpp>

#include <stdint.h>

namespace libgb {
using InterruptCallback = void (*)(void);
enum class Interrupt_t : int8_t { vblank, lcd, timer, serial, joypad };
struct Interrupt {
  static constexpr auto vblank = Constant<Interrupt_t::vblank>{};
  static constexpr auto lcd = Constant<Interrupt_t::lcd>{};
  static constexpr auto timer = Constant<Interrupt_t::timer>{};
  static constexpr auto serial = Constant<Interrupt_t::serial>{};
  static constexpr auto joypad = Constant<Interrupt_t::joypad>{};
};

namespace impl {
extern volatile InterruptCallback vblank_interrupt_callback;
extern volatile InterruptCallback lcd_status_interrupt_callback;
extern volatile InterruptCallback timer_interrupt_callback;
extern volatile InterruptCallback serial_interrupt_callback;
extern volatile InterruptCallback input_interrupt_callback;
} // namespace impl

inline auto enable_interrupts() -> void { asm volatile("ei" ::: "memory"); }
inline auto disable_interrupts() -> void { asm volatile("di" ::: "memory"); }
inline auto halt() -> void { asm volatile("halt"); }

inline auto enable_vblank_interrupt(InterruptCallback callback) -> void {
  impl::vblank_interrupt_callback = callback;
  arch::registers::set_interrupt_enable_vblank(true);
}

inline auto enable_lcd_status_interrupt(InterruptCallback callback) -> void {
  impl::lcd_status_interrupt_callback = callback;
  arch::registers::set_interrupt_enable_lcd(true);
}

inline auto enable_timer_interrupt(InterruptCallback callback) -> void {
  impl::timer_interrupt_callback = callback;
  arch::registers::set_interrupt_enable_timer(true);
}

inline auto enable_serial_interrupt(InterruptCallback callback) -> void {
  impl::serial_interrupt_callback = callback;
  arch::registers::set_interrupt_enable_serial(true);
}

inline auto enable_joypad_interrupt(InterruptCallback callback) -> void {
  impl::input_interrupt_callback = callback;
  arch::registers::set_interrupt_enable_joypad(true);
}

template <Interrupt_t interrupt>
inline auto enable_interrupt(Constant<interrupt>, InterruptCallback callback)
    -> void {
  switch (interrupt) {
  case Interrupt::vblank:
    enable_vblank_interrupt(callback);
    break;
  case Interrupt::lcd:
    enable_lcd_status_interrupt(callback);
    break;
  case Interrupt::timer:
    enable_timer_interrupt(callback);
    break;
  case Interrupt::serial:
    enable_serial_interrupt(callback);
    break;
  case Interrupt::joypad:
    enable_joypad_interrupt(callback);
    break;
  }
}

template <Interrupt_t interrupt> inline auto wait_for_interrupt() -> void {
  static volatile bool has_seen_interrupt;

  has_seen_interrupt = false;
  enable_interrupt(Constant<interrupt>{}, [] { has_seen_interrupt = true; });

  while (not has_seen_interrupt) {
    halt();
  }
}

} // namespace libgb
