#pragma once

#include <stddef.h>
#include <stdint.h>

namespace libgb {
using nullptr_t = decltype(nullptr);

template <typename To, typename From>
concept is_convertible = requires(From from) { static_cast<To>(from); };

template <typename T1, typename T2> struct is_same_t {
  static constexpr auto value = false;
};

template <typename T> struct is_same_t<T, T> {
  static constexpr auto value = true;
};

template <typename T1, typename T2>
concept is_same = is_same_t<T1, T2>::value;

template <typename T>
concept is_enum = __is_enum(T);

template <is_enum Enum> using underlying_type = __underlying_type(Enum);

template <typename T> consteval auto declvalue() -> T {
  static_assert(false, "unreachable");
}

template <typename T> struct identity_t {
  using Type = T;
};

template <typename T> using identity = identity_t<T>::Type;

template <typename T> struct remove_cv_t {
  using Type = T;
};

template <typename T> struct remove_cv_t<T const> {
  using Type = T;
};

template <typename T> struct remove_cv_t<T volatile> {
  using Type = T;
};

template <typename T> struct remove_cv_t<T const volatile> {
  using Type = T;
};

template <typename T> using remove_cv = remove_cv_t<T>::Type;

template <typename T> struct remove_ref_t {
  using Type = T;
};

template <typename T> struct remove_ref_t<T &> {
  using Type = T;
};

template <typename T> struct remove_ref_t<T &&> {
  using Type = T;
};

template <typename T> using remove_ref = remove_ref_t<T>::Type;

template <typename T> using remove_cv_ref = remove_cv<remove_ref<T>>;

template <typename T>
using element_type = remove_ref<decltype(*declvalue<T>().begin())>;

template <typename T> struct is_signed_integer_t {
  static constexpr auto value = false;
};

template <> struct is_signed_integer_t<int8_t> {
  static constexpr auto value = true;
};

template <> struct is_signed_integer_t<int16_t> {
  static constexpr auto value = true;
};

template <> struct is_signed_integer_t<int32_t> {
  static constexpr auto value = true;
};

template <> struct is_signed_integer_t<int64_t> {
  static constexpr auto value = true;
};

template <typename T>
concept is_signed_integer = is_signed_integer_t<T>::value;

template <typename T> struct is_unsigned_integer_t {
  static constexpr auto value = false;
};

template <> struct is_unsigned_integer_t<uint8_t> {
  static constexpr auto value = true;
};

template <> struct is_unsigned_integer_t<uint16_t> {
  static constexpr auto value = true;
};

template <> struct is_unsigned_integer_t<uint32_t> {
  static constexpr auto value = true;
};

template <> struct is_unsigned_integer_t<uint64_t> {
  static constexpr auto value = true;
};

template <typename T>
concept is_unsigned_integer = is_unsigned_integer_t<T>::value;

template <typename T>
concept is_integral = is_signed_integer<T> || is_unsigned_integer<T>;

template <typename T>
concept is_iterator = requires(T t) {
  ++t;
  *t;
};

template <typename T>
concept is_iterable = requires(T t) {
  t.begin() == t.end();
  t.begin() != t.end();
};

template <typename T>
concept is_collection = is_iterable<T> && requires(T t) {
  (void)t.size();
  (void)t[0];
};

template <typename T> struct is_type_list_t {
  static constexpr auto value = false;
};

template <template <typename...> typename List, typename... Ts>
struct is_type_list_t<List<Ts...>> {
  static constexpr auto value = true;
};

template <typename T>
concept is_type_list = is_type_list_t<T>::value;

template <typename T> struct is_stateless_t {
  struct is_stateless_detector : T {
    char _;
  };
  static constexpr auto value = sizeof(is_stateless_detector) == sizeof(char);
};

template <typename T>
concept is_stateless = is_stateless_t<T>::value;

template <typename T>
concept is_statefull = not is_stateless<T>;

template <bool condition, typename OnTrue, typename OnFalse> struct if_c_t;

template <typename OnTrue, typename OnFalse>
struct if_c_t<true, OnTrue, OnFalse> {
  using Type = OnTrue;
};

template <typename OnTrue, typename OnFalse>
struct if_c_t<false, OnTrue, OnFalse> {
  using Type = OnFalse;
};

template <bool condition, typename OnTrue, typename OnFalse>
using if_c = if_c_t<condition, OnTrue, OnFalse>::Type;

} // namespace libgb
