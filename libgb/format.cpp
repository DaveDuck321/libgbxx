#include <libgb/format.hpp>
#include <libgb/serial.hpp>

auto libgb::impl::print_until_next_format_arg(char const *fmt) -> char const * {
  // The format specifiers have been rewritten from {...} -> format_separator
  // They are now only a single byte.

  for (; *fmt != format_separator; fmt += 1) {
    serial_write_char(*fmt);
  }

  // Skip past the format specifier. This is completely safe since we have
  // statically checked that the number of arguments matches the number of
  // specifiers.
  return fmt + 1;
}
