#pragma once

#include <libgb/arch/enums.hpp>
#include <libgb/arch/registers.hpp>
#include <libgb/interrupts.hpp>

namespace libgb {
// Prevents all bus VRAM bus contention issues while in scope.
// Use when loading large amounts of data to the VRAM/ tilemap. Ideally, this
// should only be used to update inactive tile data tilemap is already being
// rendered.
struct ScopedVRAMGuard {
  static auto on_oam_callback() -> void {
    arch::set_interrupt_enable_lcd(false);
    enable_interrupts();
    wait_for_interrupt<libgb::Interrupt::vblank>();

    // We're safe to write into VRAM, fire again when we're out of time
    // The LCD callback already points to us
    arch::set_interrupt_flag_lcd(false);
    arch::set_interrupt_enable_lcd(true);
  }

  ScopedVRAMGuard() {
    // We don't know our current lcd status, pretend we just entered OAM (which
    // would be the worst case scenario);
    set_lcd_interrupt_condition(libgb::LCDInterruptCondition::oam_scan);

    // Wait for a vblank to clear any stale interrupts
    wait_for_interrupt<libgb::Interrupt::vblank>();
    impl::lcd_status_interrupt_callback = on_oam_callback;
    on_oam_callback();
  }

  ScopedVRAMGuard(ScopedVRAMGuard const &) = delete;
  auto operator=(ScopedVRAMGuard const &) -> ScopedVRAMGuard & = delete;

  ~ScopedVRAMGuard() { disable_lcd_status_interrupt(); }
};
} // namespace libgb
