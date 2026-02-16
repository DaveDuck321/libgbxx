#pragma once
#include <libgb/format.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/assert.hpp>
#include <libgb/std/fixed_vector.hpp>

namespace libgb {
namespace impl {
template <typename T> consteval auto parse_type_name() {
  libgb::FixedVector<char, 4096> result;

  char const *prefix = "auto libgb::impl::parse_type_name() [T = ";
  char const *function_name = __PRETTY_FUNCTION__;

  size_t offset = 0;
  while (prefix[offset] != '\0') {
    assert(function_name[offset] == prefix[offset]);
    offset += 1;
  }

  {
    // Trim the (mostly) redundant anonymous namespace label
    char const *anonymous_namespace = "(anonymous namespace)::";
    for (size_t lookahead = 0;; lookahead += 1) {
      if (anonymous_namespace[lookahead] == '\0') {
        // Commit: trim prefix
        offset += lookahead;
        break;
      }

      if (function_name[offset + lookahead] == '\0' ||
          function_name[offset + lookahead] != anonymous_namespace[lookahead]) {
        // Lookhead failed, continue parsing normally
        break;
      }
    }
  }

  size_t open_brackets = 1;
  while (true) {
    assert(function_name[offset] != '\0');
    if (function_name[offset] == '[') {
      open_brackets += 1;
    }

    if (function_name[offset] == ']') {
      open_brackets -= 1;
    }

    if (open_brackets == 0) {
      result.push_back('\0');
      return result;
    }
    result.push_back(function_name[offset]);
    offset += 1;
  }
}
} // namespace impl
template <typename T> auto type_name() -> char const * {
  static constexpr auto name_array =
      libgb::to_array<impl::parse_type_name<T>()>();
  return name_array.data();
}

} // namespace libgb
