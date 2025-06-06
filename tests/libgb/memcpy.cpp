// RUN: $GAMEBOY_EMULATOR_PATH/emulate.out $GBLIB_BUILD_DIR/memcpy.out \
// RUN:   | FileCheck %s -check-prefix=CHECK

#include <libgb/std/array.hpp>
#include <libgb/std/memcpy.hpp>

#include <stdint.h>

libgb::Array<uint8_t, 10> small;
libgb::Array<uint8_t, 40> medium;
libgb::Array<uint8_t, 128> large;
libgb::Array<uint8_t, 512> very_large;

libgb::Array<uint8_t, 512> buffer;
volatile uint16_t sentinel = 0xEFEFU;

auto check_result() -> int {
  for (size_t i = 0; i < buffer.size(); i += 1) {
    if (i < small.size()) {
      if (buffer[i] != 1) {
        return 1;
      }
    } else if (i < medium.size()) {
      if (buffer[i] != 2) {
        return 2;
      }
    } else if (i < large.size()) {
      if (buffer[i] != 3) {
        return 3;
      }
    } else if (i < very_large.size()) {
      if (buffer[i] != 4) {
        return 4;
      }
    } else {
      return 5;
    }
  }
  return sentinel ^ 0xEFEF;
}

int main() {
  asm volatile("debugtrap" ::: "memory");
  libgb::memset(small.data(), 1, small.size());
  // CHECK: Cycles since last: 86
  asm volatile("debugtrap" ::: "memory");

  libgb::memset(medium.data(), 2, medium.size());
  // CHECK: Cycles since last: 266
  asm volatile("debugtrap" ::: "memory");

  libgb::memset(large.data(), 3, large.size());
  // CHECK: Cycles since last: 794
  asm volatile("debugtrap" ::: "memory");

  libgb::memset(very_large.data(), 4, very_large.size());
  // CHECK: Cycles since last: 3110
  asm volatile("debugtrap" ::: "memory");

  libgb::memcpy(buffer.data(), very_large.data(), very_large.size());
  // CHECK: Cycles since last: 5158
  asm volatile("debugtrap" ::: "memory");

  libgb::memcpy(buffer.data(), large.data(), large.size());
  // CHECK: Cycles since last: 1306
  asm volatile("debugtrap" ::: "memory");

  libgb::memcpy(buffer.data(), medium.data(), medium.size());
  // CHECK: Cycles since last: 426
  asm volatile("debugtrap" ::: "memory");

  libgb::memcpy(buffer.data(), small.data(), small.size());
  // CHECK: Cycles since last: 126
  asm volatile("debugtrap" ::: "memory");

  // CHECK: hl=0000
  return check_result();
}
