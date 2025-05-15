#include <libgb/interrupts.hpp>

extern "C" {
libgb::InterruptCallback __libgb_handle_vblank_interrupt;
libgb::InterruptCallback __libgb_handle_lcd_status_interrupt;
libgb::InterruptCallback __libgb_handle_timer_interrupt;
libgb::InterruptCallback __libgb_handle_serial_interrupt;
libgb::InterruptCallback __libgb_handle_input_interrupt;
}

// Shim layer to namespace the globals without having to write mangled names in
// assembly
namespace libgb::impl {
[[gnu::alias(
    "__libgb_handle_vblank_interrupt")]] extern volatile InterruptCallback
    vblank_interrupt_callback;

[[gnu::alias(
    "__libgb_handle_lcd_status_interrupt")]] extern volatile InterruptCallback
    lcd_status_interrupt_callback;

[[gnu::alias(
    "__libgb_handle_timer_interrupt")]] extern volatile InterruptCallback
    timer_interrupt_callback;

[[gnu::alias(
    "__libgb_handle_serial_interrupt")]] extern volatile InterruptCallback
    serial_interrupt_callback;

[[gnu::alias(
    "__libgb_handle_input_interrupt")]] extern volatile InterruptCallback
    input_interrupt_callback;
} // namespace libgb::impl
