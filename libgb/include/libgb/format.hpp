#pragma once

#include <libgb/serial.hpp>

namespace libgb {
namespace impl {
auto print_until_next_format_arg(char const *fmt) -> char const *;

enum class FormatType { unspecified, escape };
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
  for (size_t i = 0; i < new_size; i += 1) {
    new_fmt.m_data[i] = fmt.m_data[i];
  }

  if constexpr (new_format_args != 0) {
    for (size_t i = 0; i < new_format_args; i += 1) {
      new_fmt.m_format_types[i] = fmt.m_format_types[i];
    }
  }
  return new_fmt;
}

template <typename... Args>
inline auto unchecked_println(char const *fmt, Args... args) -> void {
  auto print_next_section = [&] [[gnu::always_inline]] (auto arg) {
    fmt = print_until_next_format_arg(fmt);
    serial_write(arg);
  };
  (print_next_section(args), ...);
  print_until_next_format_arg(fmt);
  serial_write_char('\n');
}

} // namespace impl

template <impl::FormatString fmt, typename... Args>
[[gnu::always_inline]] inline auto println(Args... args) -> void {
  static constexpr auto fmt_string =
      impl::shrink_wrap_format_string<fmt>().m_data;
  static_assert(fmt.m_args == sizeof...(args),
                "println arguments do not match format specifier");
  impl::unchecked_println(fmt_string.data(), args...);
}
} // namespace libgb
