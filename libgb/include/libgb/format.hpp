#pragma once

#include <libgb/serial.hpp>
#include <libgb/std/algorithms.hpp>
#include <libgb/std/traits.hpp>

namespace libgb {
namespace impl {
auto print_until_next_format_arg(char const *fmt) -> char const *;

enum class FormatType { unspecified, escape, no_prefix };
static constexpr char format_separator = '\0';

template <size_t N, size_t AllocatedSpecifiers> struct FormatString {
  Array<char, N> m_data = {};
  Array<FormatType, AllocatedSpecifiers> m_format_types = {};
  size_t m_length = 0;
  size_t m_args = 0;

  constexpr auto parse_format_specifier(char const c_str[N], size_t &index)
      -> FormatType {
    assert(c_str[index++] == '{');
    if (c_str[index] == '{') {
      return FormatType::escape;
    }
    if (c_str[index] == '#') {
      index += 1;
      assert(c_str[index++] == '}');
      return FormatType::no_prefix;
    }

    assert(c_str[index++] == '}');
    return FormatType::unspecified;
  }

  consteval FormatString() = default;

  // c-str constructor N == strlen(cstr)
  consteval FormatString(char const c_str[N]) {
    for (size_t input_index = 0; input_index < N - 1;) {
      if (c_str[input_index] == '{') {
        // The parser will advance input_index past the specifier
        auto specifier = parse_format_specifier(c_str, input_index);
        if (specifier != FormatType::escape) {
          m_format_types[m_args++] = specifier;
          m_data[m_length++] = format_separator;
          continue;
        }
      }
      m_data[m_length++] = c_str[input_index++];
    }
    m_data[m_length++] = format_separator;
  }
};

template <size_t N> FormatString(char const (&)[N]) -> FormatString<N, N>;

template <impl::FormatString fmt>
consteval auto shrink_wrap_format_string()
    -> FormatString<fmt.m_length, fmt.m_args> {
  constexpr auto new_size = fmt.m_length;
  constexpr auto new_format_args = fmt.m_args;

  FormatString<new_size, new_format_args> new_fmt;
  new_fmt.m_data = slice<0, new_size>(fmt.m_data);
  new_fmt.m_format_types = slice<0, new_format_args>(fmt.m_format_types);
  return new_fmt;
}

template <auto format_types, typename... Args>
inline auto unchecked_print(char const *fmt, Args... args) -> void {
  auto print_next_section =
      [&]<typename Arg> [[gnu::always_inline]] (size_t index, Arg arg) {
        fmt = print_until_next_format_arg(fmt);
        if constexpr (is_same<Arg, uint8_t> || is_same<Arg, uint16_t>) {
          if (format_types[index] == FormatType::no_prefix) {
            serial_write(arg, /*prefix=*/false);
            return;
          }
        }
        serial_write(arg);
      };

  size_t index = 0;
  (print_next_section(index++, args), ...);
  print_until_next_format_arg(fmt);
}

} // namespace impl

template <impl::FormatString fmt, typename... Args>
[[gnu::always_inline]] inline auto print(Args... args) -> void {
  static constexpr auto fmt_string =
      impl::shrink_wrap_format_string<fmt>().m_data;
  static_assert(fmt.m_args == sizeof...(args),
                "println arguments do not match format specifier");
  impl::unchecked_print<fmt.m_format_types>(fmt_string.data(), args...);
}

template <impl::FormatString fmt, typename... Args>
[[gnu::always_inline]] inline auto println(Args... args) -> void {
  print<fmt>(args...);
  serial_write_char('\n');
}
} // namespace libgb
