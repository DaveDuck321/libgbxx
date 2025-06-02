#include <libgb/std/memcpy.hpp>
#include <stdint.h>

auto memcpy(void *dst, void const *src, size_t count) -> void {
  char const *src_ptr = (char const *)src;
  char *dst_ptr = (char *)dst;
  while (count-- != 0) {
    *(dst_ptr++) = *(src_ptr++);
  }
}

auto memset(void *dst, int byte, size_t count) -> void {
  char *dst_ptr = (char *)dst;
  while (count-- != 0) {
    *(dst_ptr++) = static_cast<uint8_t>(byte);
  }
}
