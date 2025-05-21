#pragma once

#include <libgb/arch/enums.hpp>
#include <libgb/arch/registers.hpp>
#include <libgb/std/meta.hpp>

#include <libgb/std/array.hpp>
#include <libgb/std/assert.hpp>
#include <libgb/std/string_view.hpp>

#include <stddef.h>
#include <stdint.h>

namespace libgb {
auto serial_write(char const *to_write) -> void;
auto serial_write(StringView) -> void;
auto serial_write(uint16_t) -> void;
auto serial_write_char(char) -> void;
} // namespace libgb
