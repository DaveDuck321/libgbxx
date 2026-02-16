// RUN: $GAMEBOY_EMULATOR_PATH/emulate.out $GBLIB_BUILD_DIR/type_name.out \
// RUN:   | FileCheck %s -check-prefix=CHECK

#include <libgb/format.hpp>
#include <libgb/interrupts.hpp>
#include <libgb/std/type_name.hpp>

struct Type1 {};

namespace {
struct Type2 {};
} // namespace

namespace my_namespace::nested {
struct Type3 {};
} // namespace my_namespace::nested

template <typename T> struct Type4 {};

using namespace my_namespace::nested;

int main() {
  libgb::enable_interrupts();
  libgb::println<"{}">(libgb::type_name<Type1>());
  // CHECK: Type1

  libgb::println<"{}">(libgb::type_name<Type2>());
  // CHECK: Type2

  libgb::println<"{}">(libgb::type_name<Type3>());
  // CHECK: my_namespace::nested::Type3

  libgb::println<"{}">(libgb::type_name<Type4<Type1>>());
  // CHECK: Type4<Type1>

  return 0;
}
