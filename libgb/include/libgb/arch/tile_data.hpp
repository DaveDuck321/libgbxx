#pragma once

#include <libgb/arch/tile.hpp>
#include <libgb/std/enum.hpp>
#include <libgb/std/math.hpp>
#include <libgb/std/memcpy.hpp>

#include <stdint.h>

namespace libgb {
enum class TileAddress : uint16_t {};
enum class TileIndex : uint8_t {};
enum class TileAddressingMode : uint8_t {
  object,
  bg_window_unsigned = object,
  bg_window_signed,
};

namespace impl {
constexpr auto tile_address_offset(TileIndex index, TileAddressingMode mode)
    -> uint16_t {
  switch (mode) {
  case TileAddressingMode::bg_window_unsigned:
    return sizeof(arch::Tile) * zext(+index);
  case TileAddressingMode::bg_window_signed:
    return sizeof(arch::Tile) * sext(+index);
  }
}
} // namespace impl

constexpr auto tile_address(TileIndex index, TileAddressingMode mode)
    -> TileAddress {
  auto offset = impl::tile_address_offset(index, mode);
  switch (mode) {
  case TileAddressingMode::bg_window_unsigned:
    return TileAddress{static_cast<uint16_t>(0x8000U + offset)};
  case TileAddressingMode::bg_window_signed:
    return TileAddress{static_cast<uint16_t>(0x9000U + offset)};
  }
}

inline auto set_tile_data(TileAddress dst, arch::Tile const &src) -> void {
  libgb::memcpy((uint8_t volatile *)dst, (const uint8_t *)&src,
                sizeof(arch::Tile));
}
} // namespace libgb
