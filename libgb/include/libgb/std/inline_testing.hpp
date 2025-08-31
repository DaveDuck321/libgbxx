#pragma once

#include "macro.hpp"
#include "traits.hpp"

#include <stddef.h>

#ifndef GB_NO_INLINE_TESTS
namespace libgb::testing {
class TestResult {
  char const *m_error_msg; // nullptr on success
  size_t m_error_msg_size;

public:
  constexpr TestResult(libgb::nullptr_t)
      : m_error_msg{nullptr}, m_error_msg_size{0} {}

  template <size_t Size>
  constexpr TestResult(const char (&error_message)[Size])
      : m_error_msg{error_message}, m_error_msg_size{Size - 1} {}

  constexpr auto did_pass() const -> bool { return m_error_msg == nullptr; }

  // Magic endpoints required for static_assert formatting
  constexpr auto data() const -> char const * {
    if (m_error_msg == nullptr) {
      // We won't actually be printing the message but clang complains anyway if
      // it isn't a valid string.
      return "";
    } else {
      return m_error_msg;
    }
  }
  constexpr auto size() const -> size_t { return m_error_msg_size; }
};

struct A {};
struct B {};
struct C {};
} // namespace libgb::testing

#define PASS()                                                                 \
  return ::libgb::testing::TestResult { nullptr }

#define FAIL(reason)                                                           \
  return ::libgb::testing::TestResult {                                        \
    reason ": " __FILE__ ":" STRINGIFY(__LINE__)                               \
  }

#define CHECK(args...)                                                         \
  do {                                                                         \
    if (!(args)) {                                                             \
      FAIL("Check failed");                                                    \
    }                                                                          \
  } while (false)

#define INLINE_TEST(lambda...)                                                 \
  static_assert([] {                                                           \
    constexpr auto result = lambda();                                          \
    static_assert(result.did_pass(), result);                                  \
    return true;                                                               \
  }());
#else
#define LIBGB_INLINE_TEST(lambda...)
#endif
