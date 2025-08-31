#pragma once

#include <libgb/std/assert.hpp>
#include <libgb/std/meta.hpp>
#include <libgb/std/new.hpp>
#include <libgb/std/traits.hpp>
#include <libgb/std/utility.hpp>

#include <stdint.h>

namespace libgb {
namespace impl {
template <typename... T> union VariadicUnion;

template <> union VariadicUnion<> {};

template <typename T1, typename... Ts> union VariadicUnion<T1, Ts...> {
  union {
    T1 option_1;
    VariadicUnion<Ts...> option_2;
  };

  constexpr VariadicUnion() : option_2{} {}

  template <typename T>
    requires(is_same<remove_cv_ref<T>, T1>)
  constexpr VariadicUnion(T &&t) : option_1{forward<T>(t)} {}

  template <typename T>
    requires(!is_same<remove_cv_ref<T>, T1>)
  constexpr VariadicUnion(T &&t) : option_2{forward<T>(t)} {}

  constexpr VariadicUnion(VariadicUnion const &) = default;
  constexpr VariadicUnion(VariadicUnion &&) = default;
  constexpr auto operator=(VariadicUnion const &) -> VariadicUnion & = default;
  constexpr auto operator=(VariadicUnion &&) -> VariadicUnion & = default;
  constexpr ~VariadicUnion() { /* Destructing handled by Variant */ }

  template <size_t index, typename Self>
  constexpr auto get(this Self &&self) -> auto & {
    if constexpr (index == 0) {
      return self.option_1;
    } else {
      return self.option_2.template get<index - 1>();
    }
  }
};

} // namespace impl

template <typename... Ts> class Variant {
  using StorageType = impl::VariadicUnion<Ts...>;
  StorageType m_storage;
  uint8_t m_index;

  template <typename T> static consteval auto get_type_index() -> uint8_t {
    uint8_t index = 0;
    bool found = false;
    (
        [&]() mutable {
          if (found) {
            return;
          }

          if constexpr (is_same<T, Ts>) {
            found = true;
          } else {
            index += 1;
          }
        }(),
        ...);
    return index;
  }

  template <size_t index, typename Fn>
  constexpr auto visit_impl(Fn &&fn) -> decltype(auto) {
    if constexpr (index != sizeof...(Ts) - 1) {
      if (index == m_index) {
        return fn(get<index>());
      } else {
        return visit_impl<index + 1>(forward<Fn>(fn));
      }
    } else {
      return fn(get<index>());
    }
  }

public:
  template <typename T>
    requires((is_same<remove_cv_ref<T>, Ts> || ...))
  constexpr Variant(T &&t)
      : m_storage{forward<T>(t)}, m_index{get_type_index<remove_cv_ref<T>>()} {}

  constexpr Variant(Variant const &other)
      : m_storage{other}, m_index{other.m_index} {}

  constexpr Variant(Variant &&other)
      : m_storage{move(other)}, m_index{other.m_index} {}

  constexpr auto operator=(Variant const &other) -> Variant & {
    if (m_index == other.m_index) {
      visit([&](auto &active_member) {
        using MemberType = remove_cv_ref<decltype(active_member)>;
        active_member = other.get<MemberType>();
      });
    } else {
      // Types are incompatible, destruct our old type, emplace into storage
      visit([](auto &active_member) {
        active_member.~decltype(active_member)();
      });

      m_index = other.m_index;
      other.visit([&](auto &other_active_member) {
        using OtherType = remove_cv_ref<decltype(other_active_member)>;
        auto &our_member = get<OtherType>();
        m_storage = other_active_member;
      });
    }
    return *this;
  }

  constexpr auto operator=(Variant &&other) -> Variant & {
    if (m_index == other.m_index) {
      visit([&](auto &active_member) {
        using MemberType = remove_cv_ref<decltype(active_member)>;
        active_member = move(other.get<MemberType>());
      });
    } else {
      // Types are incompatible, destruct our old type, emplace into storage
      visit([](auto &active_member) {
        using MemberType = remove_cv_ref<decltype(active_member)>;
        active_member.~MemberType();
      });

      m_storage.~StorageType();

      m_index = other.m_index;
      other.visit([&](auto &other_active_member) {
        using OtherType = remove_cv_ref<decltype(other_active_member)>;
        auto &our_member = get<OtherType>();
        libgb::construct_at(&m_storage, move(other_active_member));
      });
    }
    return *this;
  }

  template <uint8_t index, typename Self>
    requires(index < sizeof...(Ts))
  constexpr auto get(this Self &&self) -> auto & {
    assert(self.m_index == index);
    return self.m_storage.template get<index>();
  }

  template <typename T, typename Self>
    requires(is_same<T, Ts> || ...)
  constexpr auto get(this Self &&self) -> auto & {
    constexpr auto type_index = get_type_index<T>();
    return self.template get<type_index>();
  }

  template <typename Self, typename Fn>
  constexpr auto visit(this Self &&self, Fn &&fn) -> decltype(auto) {
    return self.template visit_impl</*index=*/0>(forward<Fn>(fn));
  }
};
} // namespace libgb

#include "inline_testing.hpp"

INLINE_TEST([] {
  using namespace libgb::testing;

  libgb::Variant<A, B, C> variant = A{};

  {
    auto res = variant.template get<A>();
    CHECK(libgb::is_same<decltype(res), A>);
  }

  {
    variant = B{};
    auto res = variant.template get<B>();
    CHECK(libgb::is_same<decltype(res), B>);
  }

  {
    auto copy = variant;
    // copy = C{};

    // auto res_1 = variant.template get<B>();
    // CHECK(libgb::is_same<decltype(res_1), B>);

    // auto res_2 = copy.template get<C>();
    // CHECK(libgb::is_same<decltype(res_2), C>);
  }
  PASS();
});
