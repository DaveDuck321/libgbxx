#pragma once

#include <libgb/arch/registers.hpp>

#include <stdint.h>

namespace libgb {
using InterruptCallback = void (*)(void);
enum class Interrupt : int8_t { vblank, lcd, timer, serial, joypad };

namespace impl {
extern volatile InterruptCallback vblank_interrupt_callback;
extern volatile InterruptCallback lcd_status_interrupt_callback;
extern volatile InterruptCallback timer_interrupt_callback;
extern volatile InterruptCallback serial_interrupt_callback;
extern volatile InterruptCallback input_interrupt_callback;

auto default_interrupt_callback() -> void;
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

inline auto disable_vblank_interrupt() -> void {
  arch::registers::set_interrupt_enable_vblank(false);
  impl::vblank_interrupt_callback = impl::default_interrupt_callback;
}

inline auto disable_lcd_status_interrupt() -> void {
  arch::registers::set_interrupt_enable_lcd(false);
  impl::lcd_status_interrupt_callback = impl::default_interrupt_callback;
}

inline auto disable_timer_interrupt() -> void {
  arch::registers::set_interrupt_enable_timer(false);
  impl::timer_interrupt_callback = impl::default_interrupt_callback;
}

inline auto disable_serial_interrupt() -> void {
  arch::registers::set_interrupt_enable_serial(false);
  impl::serial_interrupt_callback = impl::default_interrupt_callback;
}

inline auto disable_joypad_interrupt() -> void {
  arch::registers::set_interrupt_enable_joypad(false);
  impl::input_interrupt_callback = impl::default_interrupt_callback;
}

template <Interrupt interrupt>
inline auto enable_interrupt(InterruptCallback callback) -> void {
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

template <Interrupt interrupt> inline auto disable_interrupt() -> void {
  switch (interrupt) {
  case Interrupt::vblank:
    disable_vblank_interrupt();
    break;
  case Interrupt::lcd:
    disable_lcd_status_interrupt();
    break;
  case Interrupt::timer:
    disable_timer_interrupt();
    break;
  case Interrupt::serial:
    disable_serial_interrupt();
    break;
  case Interrupt::joypad:
    disable_joypad_interrupt();
    break;
  }
}

template <Interrupt interrupt> struct InterruptScope {
  explicit InterruptScope(InterruptCallback callback) {
    enable_interrupt<interrupt>(callback);
  }
  InterruptScope(InterruptScope const &) = delete;
  auto operator=(InterruptScope const &) -> InterruptScope & = delete;
  ~InterruptScope() { disable_interrupt<interrupt>(); }
};

template <Interrupt interrupt> inline auto wait_for_interrupt() -> void {
  static volatile bool has_seen_interrupt;
  has_seen_interrupt = false;

  InterruptScope<interrupt> handler([] { has_seen_interrupt = true; });
  while (not has_seen_interrupt) {
    halt();
  }
}
} // namespace libgb
