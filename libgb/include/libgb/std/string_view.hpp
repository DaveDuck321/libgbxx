#pragma once

#include <stddef.h>

struct StringView {
  char const *data;
  size_t size;
};
