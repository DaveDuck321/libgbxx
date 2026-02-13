#include <libgb/gameloop.hpp>

namespace libgb::gameloop {
namespace impl {
uint8_t tick_count = 0;
uint8_t dropped_frames = 0;
bool is_running = true;
} // namespace impl

auto run(Callable on_tick, Callable on_vblank) -> int {
  bool is_running = true;
  while (is_running) {
    if (is_interrupt_pending<libgb::Interrupt::vblank>()) {
      impl::dropped_frames += 1;

      // Discard the stale interrupt
      libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
    }

    libgb::wait_for_interrupt<libgb::Interrupt::vblank>();
    on_vblank();

    libgb::read_inputs();
    on_tick();

    impl::tick_count += 1;
  }
  return 0;
}
} // namespace libgb::gameloop
