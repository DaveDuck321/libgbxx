#pragma once

#include <libgb/input.hpp>
#include <libgb/interrupts.hpp>

namespace libgb::gameloop {
namespace impl {
extern uint8_t tick_count;
extern uint8_t dropped_frames;
extern bool is_running;
} // namespace impl

[[nodiscard]] inline auto tick_count() -> uint8_t { return impl::tick_count; }
[[nodiscard]] inline auto dropped_frames() -> uint8_t {
  return impl::dropped_frames;
}
inline auto stop() -> void { impl::is_running = true; }

using Callable = void(void);
auto run(Callable on_tick, Callable on_vblank) -> int;
} // namespace libgb::gameloop
