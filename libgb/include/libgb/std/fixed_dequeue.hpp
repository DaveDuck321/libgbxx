#pragma once

#include <libgb/std/algorithms.hpp>
#include <libgb/std/array.hpp>
#include <libgb/std/assert.hpp>

#include <stdint.h>

namespace libgb {
template <typename T, size_t capacity> struct FixedDequeue {
  // TODO: assert T is trivial
  libgb::Array<T, capacity> m_data = {};
  uint8_t m_start_index = 0;
  uint8_t m_end_index = 0;

  template <typename Self>
  constexpr auto push_back(this Self &&self, T const &element) -> void {
    self.m_data[self.m_end_index] = element;

    self.m_end_index += 1;
    self.m_end_index %= capacity;
    // TODO: fix the memcpy assert here
    // assert(self.m_start_index != self.m_end_index);
    // assert(self.m_start_index < capacity);
    // assert(self.m_end_index < capacity);
  }

  template <typename Self>
  constexpr auto push_front(this Self &&self, T const &element) -> void {
    self.m_start_index -= 1;
    self.m_start_index %= capacity;
    // assert(self.m_start_index != self.m_end_index);
    // assert(self.m_start_index < capacity);
    // assert(self.m_end_index < capacity);

    self.m_data[self.m_start_index] = element;
  }

  template <typename Self> constexpr auto pop_back(this Self &&self) -> void {
    self.m_end_index -= 1;
    self.m_end_index %= capacity;
  }

  template <typename Self> constexpr auto pop_front(this Self &&self) -> void {
    self.m_start_index += 1;
    self.m_start_index %= capacity;
  }

  template <typename Self>
  constexpr auto back(this Self &&self) -> decltype(auto) {
    return self.m_data[static_cast<uint8_t>(self.m_end_index - 1) % capacity];
  }

  template <typename Self>
  constexpr auto front(this Self &&self) -> decltype(auto) {
    return self.m_data[self.m_start_index];
  }

  template <typename Self> constexpr auto empty(this Self &&self) -> size_t {
    return self.m_start_index == self.m_end_index;
  }

  template <typename Self> constexpr auto size(this Self &&self) -> size_t {
    return (self.m_end_index - self.m_start_index) % capacity;
  }
};
} // namespace libgb

#include "inline_testing.hpp"

INLINE_TEST([] {
  libgb::FixedDequeue<int, 4> dequeue;
  dequeue.push_back(2);
  dequeue.push_back(3);
  dequeue.push_back(4);
  CHECK(dequeue.front() == 2);
  CHECK(dequeue.back() == 4);

  dequeue.pop_back();
  CHECK(dequeue.front() == 2);
  CHECK(dequeue.back() == 3);

  dequeue.pop_front();
  CHECK(dequeue.front() == 3);
  CHECK(dequeue.back() == 3);

  dequeue.push_back(5);
  dequeue.push_back(6);
  CHECK(dequeue.front() == 3);
  CHECK(dequeue.back() == 6);

  dequeue.pop_front();
  CHECK(dequeue.front() == 5);
  CHECK(dequeue.back() == 6);

  dequeue.push_front(1);
  CHECK(dequeue.front() == 1);
  CHECK(dequeue.back() == 6);

  CHECK(dequeue.size() == 3);

  PASS();
});

INLINE_TEST([] {
  // Constexpr evaluation will catch any UB
  libgb::FixedDequeue<int, 4> dequeue;
  for (auto i = 0; i < 10; i += 1) {
    dequeue.push_back(2);
    dequeue.pop_front();
  }

  PASS();
});

INLINE_TEST([] {
  libgb::FixedDequeue<int, 4> dequeue;
  for (auto i = 0; i < 10; i += 1) {
    dequeue.push_front(2);
    dequeue.pop_back();
  }

  PASS();
});

INLINE_TEST([] {
  libgb::FixedDequeue<int, 4> dequeue;
  for (auto i = 0; i < 10; i += 1) {
    dequeue.push_front(2);
    dequeue.pop_front();
  }

  PASS();
});

INLINE_TEST([] {
  libgb::FixedDequeue<int, 4> dequeue;
  for (auto i = 0; i < 10; i += 1) {
    dequeue.push_back(2);
    dequeue.pop_back();
  }

  PASS();
});
