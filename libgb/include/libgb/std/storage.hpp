#pragma once

#include "traits.hpp"
#include <libgb/std/array.hpp>
#include <libgb/std/math.hpp>
#include <libgb/std/memcpy.hpp>
#include <libgb/std/meta.hpp>
#include <libgb/std/new.hpp>

#include <stdint.h>

namespace libgb {
namespace impl {
template <typename... T> constexpr auto max_size() -> size_t {
  size_t max_size = 1;
  (
      [&] {
        if (sizeof(T) > max_size) {
          max_size = sizeof(T);
        }
      }(),
      ...);
  return max_size;
}
} // namespace impl

struct EmptyStorage {};

template <typename... Ts> class StorageForAny {
  using PossibleTypes = TypeList<Ts...>;

  alignas(Ts...) libgb::Array<uint8_t, impl::max_size<Ts...>()> m_storage;

public:
  constexpr auto storage_ptr() -> uint8_t * { return m_storage.data(); }

  template <meta::is_contained<PossibleTypes> T> auto ptr() -> T * {
    return reinterpret_cast<T *>(storage_ptr());
  }

  template <meta::is_contained<PossibleTypes> T> auto ref() -> T & {
    return *ptr<T>();
  }

  template <meta::is_contained<PossibleTypes> T, typename... Args>
  auto emplace(Args... args) -> T & {
    return *libgb::construct_at(ptr<T>(), libgb::forward<Args>(args)...);
  }

  template <meta::is_contained<PossibleTypes> T> auto destruct() -> void {
    libgb::destroy_at(ptr<T>());
  }
};

template <typename... Ts>
using MaybeEmptyStorageForAny = if_c<meta::all<is_stateless_t, TypeList<Ts...>>,
                                     EmptyStorage, StorageForAny<Ts...>>;

template <typename T> class StorageFor {
  alignas(T) libgb::Array<uint8_t, max_size<T>()> m_storage;

public:
  auto storage_ptr() -> uint8_t * { return m_storage.data(); }
  auto ptr() -> T * { return reinterpret_cast<T *>(storage_ptr()); }
  auto ref() -> T & { return *ptr<T>(); }

  template <typename... Args> auto emplace(Args... args) -> T & {
    libgb::construct_at(ptr(), libgb::forward<Args>(args)...);
  }
  auto destruct() -> void { libgb::destroy_at(ptr()); }
};

} // namespace libgb
