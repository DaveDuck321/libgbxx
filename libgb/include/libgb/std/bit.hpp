#pragma once

namespace libgb {
template <typename U, typename V> static auto bitcast(V v) -> U {
  static_assert(sizeof(U) == sizeof(V));
  static_assert(alignof(U) == alignof(V));

  U result;
  __builtin_memcpy(&result, &v, sizeof(U));
  return result;
}
} // namespace libgb
