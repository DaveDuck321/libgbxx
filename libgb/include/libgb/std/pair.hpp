#pragma once

namespace libgb {
template <typename First, typename Second> struct Pair {
  First first;
  [[no_unique_address]] Second second;
};
} // namespace libgb
