#pragma once

#include <libgb/arch/enums.hpp>
#include <libgb/arch/registers.hpp>

namespace libgb {
auto serial_write(char const *to_write) -> void;
auto serial_write(uint16_t) -> void;

template <typename... Ts> inline auto println(Ts... ts) {
  (serial_write(ts), ...);
  serial_write("\n");
}
} // namespace libgb
