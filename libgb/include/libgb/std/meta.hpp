#pragma once

#include <libgb/std/traits.hpp>

#include <stddef.h>

namespace libgb {
template <typename... T> struct TypeList {};

template <auto... T> struct ValueList {};

template <typename T> struct Type {};

template <auto V> struct Constant {
  using Type = decltype(V);
  static constexpr Type Value = V;

  constexpr Constant() {};
  constexpr operator Type() const { return Value; }
};

namespace meta {
template <template <typename> typename MetaFn, is_type_list List>
struct apply_t;

template <template <typename> typename MetaFn,
          template <typename...> typename List, typename... Ts>
struct apply_t<MetaFn, List<Ts...>> {
  using type = List<MetaFn<Ts>...>;
};

template <template <typename> typename MetaFn, is_type_list List>
using apply = apply_t<MetaFn, List>;

template <template <typename> typename MetaFn, is_type_list List> struct any_t;
template <template <typename> typename MetaFn,
          template <typename...> typename List, typename... Ts>
struct any_t<MetaFn, List<Ts...>> {
  static constexpr auto value = (MetaFn<Ts>::value || ...);
};

template <template <typename> typename MetaFn, typename List>
concept any = any_t<MetaFn, List>::value;

template <template <typename> typename MetaFn, is_type_list List> struct all_t;
template <template <typename> typename MetaFn,
          template <typename...> typename List, typename... Ts>
struct all_t<MetaFn, List<Ts...>> {
  static constexpr auto value = (MetaFn<Ts>::value && ...);
};

template <template <typename> typename MetaFn, typename List>
concept all = all_t<MetaFn, List>::value;

template <template <typename...> typename MetaFn, typename... Params>
struct curry_t {
  template <typename ExtraParams> using Fn = MetaFn<Params..., ExtraParams>;
};

template <typename T, typename List>
concept is_contained = any<curry_t<is_same_t, T>::template Fn, List>;

template <is_type_list List> struct tail_t;
template <template <typename...> typename List, typename T1, typename... Others>
struct tail_t<List<T1, Others...>> {
  using Type = List<Others...>;
};
template <is_type_list List> using tail = tail_t<List>::Type;

template <is_type_list List> struct head_t;
template <template <typename...> typename List, typename T1, typename... Others>
struct head_t<List<T1, Others...>> {
  using Type = T1;
};
template <is_type_list List> using head = head_t<List>::Type;

template <is_type_list List1, is_type_list List2> struct concat_t;
template <template <typename...> typename List1,
          template <typename...> typename List2, typename... Items1,
          typename... Items2>
struct concat_t<List1<Items1...>, List2<Items2...>> {
  using Type = List1<Items1..., Items2...>;
};

template <is_type_list List1, is_type_list List2>
using concat = concat_t<List1, List2>::Type;

template <is_type_list List1, typename Item>
using append = concat<List1, TypeList<Item>>;

template <template <typename...> typename MetaFn, is_type_list List>
struct expand_into_t;

template <template <typename...> typename MetaFn,
          template <typename...> typename List, typename... Args>
struct expand_into_t<MetaFn, List<Args...>> {
  using Type = MetaFn<Args...>;
};

template <template <typename...> typename MetaFn, is_type_list List>
using expand_into = expand_into_t<MetaFn, List>::Type;
} // namespace meta
} // namespace libgb
