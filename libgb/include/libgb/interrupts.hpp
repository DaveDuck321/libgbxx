#pragma once

#include <libgb/arch/registers.hpp>

namespace libgb {
using InterruptCallback = void (*)(void);

namespace impl {
extern volatile InterruptCallback vblank_interrupt_callback;
extern volatile InterruptCallback lcd_status_interrupt_callback;
extern volatile InterruptCallback timer_interrupt_callback;
extern volatile InterruptCallback serial_interrupt_callback;
extern volatile InterruptCallback input_interrupt_callback;
} // namespace impl

inline auto enable_interrupts() -> void { asm volatile("ei" ::: "memory"); }
inline auto disable_interrupts() -> void { asm volatile("di" ::: "memory"); }

inline auto set_vblank_interrupt_handler() -> void {

}

} // namespace libgb
