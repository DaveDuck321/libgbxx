// RUN: $GAMEBOY_EMULATOR_PATH/emulate.out $GBLIB_BUILD_DIR/print.out \
// RUN:   | FileCheck %s -check-prefix=CHECK

#include <libgb/format.hpp>
#include <libgb/interrupts.hpp>

int main() {
  libgb::enable_interrupts();
  libgb::println<"hello world">();
  // CHECK: hello world

  libgb::println<"{}">(0x2710);
  // CHECK: $2710

  libgb::println<"{}{}">(1, 2);
  // CHECK: $0001$0002

  libgb::println<"hello {} {} world({})!">(1, 2, 3);
  // CHECK: hello $0001 $0002 world($0003)!

  libgb::println<"hello {}">("world");
  // CHECK: hello world

  libgb::println<"test {{}{{{}}">(1);
  // CHECK: test {}{$0001}

  libgb::println<"{#}">(0x2710);
  // CHECK: 2710

  return 0;
}
