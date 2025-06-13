#pragma once

#include <libgb/std/array.hpp>

#include <stdint.h>

struct Kick {
  int8_t x;
  int8_t y;
};

struct Coordinate {
  int8_t x;
  int8_t y;

  constexpr auto operator+(Coordinate other) const -> Coordinate {
    return Coordinate{(int8_t)(x + other.x), (int8_t)(y + other.y)};
  }

  constexpr auto operator-(Coordinate other) const -> Coordinate {
    return Coordinate{(int8_t)(x - other.x), (int8_t)(y - other.y)};
  }

  constexpr auto operator+(Kick other) const -> Coordinate {
    return Coordinate{(int8_t)(x + other.x), (int8_t)(y + other.y)};
  }
};

using Piece = libgb::Array<Coordinate, 4>;
using Rotations = libgb::Array<Piece, 4>;
using KickAttempts = libgb::Array<Kick, 4>;
using Kicks = libgb::Array<KickAttempts, 4>;

// Tables: https://tetris.fandom.com/wiki/SRS
static constexpr Rotations i_piece = {{
    {{{0, 2}, {1, 2}, {2, 2}, {3, 2}}},
    {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
    {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
    {{{1, 0}, {1, 1}, {1, 2}, {1, 3}}},
}};
static constexpr Rotations j_piece = {{
    {{{0, 1}, {0, 2}, {1, 1}, {2, 1}}},
    {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}},
    {{{0, 1}, {1, 1}, {2, 1}, {2, 0}}},
    {{{0, 0}, {1, 0}, {1, 1}, {1, 2}}},
}};
static constexpr Rotations l_piece = {{
    {{{0, 1}, {1, 1}, {2, 1}, {2, 2}}},
    {{{1, 0}, {2, 0}, {1, 1}, {1, 2}}},
    {{{0, 1}, {0, 0}, {1, 1}, {2, 1}}},
    {{{0, 2}, {1, 2}, {1, 1}, {1, 0}}},
}};

static constexpr Rotations o_piece = {{
    {{{1, 1}, {1, 2}, {2, 1}, {2, 2}}},
    {{{1, 1}, {1, 2}, {2, 1}, {2, 2}}},
    {{{1, 1}, {1, 2}, {2, 1}, {2, 2}}},
    {{{1, 1}, {1, 2}, {2, 1}, {2, 2}}},
}};
static constexpr Rotations s_piece = {{
    {{{0, 1}, {1, 1}, {1, 2}, {2, 2}}},
    {{{1, 2}, {1, 1}, {2, 1}, {2, 0}}},
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
    {{{0, 2}, {0, 1}, {1, 1}, {1, 0}}},
}};
static constexpr Rotations t_piece = {{
    {{{0, 1}, {1, 1}, {1, 2}, {2, 1}}},
    {{{1, 0}, {1, 1}, {1, 2}, {2, 1}}},
    {{{0, 1}, {1, 1}, {1, 0}, {2, 1}}},
    {{{0, 1}, {1, 0}, {1, 1}, {1, 2}}},
}};
static constexpr Rotations z_piece = {{
    {{{0, 2}, {1, 2}, {1, 1}, {2, 1}}},
    {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}},
    {{{0, 1}, {1, 1}, {1, 0}, {2, 0}}},
    {{{0, 0}, {0, 1}, {1, 1}, {1, 2}}},
}};

static constexpr Kicks clockwise_standard_kicks = {{
    {{{-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}},
    {{{1, 0}, {1, -1}, {0, 2}, {1, 2}}},
    {{{1, 0}, {1, 1}, {0, -2}, {1, -2}}},
    {{{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},
}};

static constexpr Kicks counter_clockwise_standard_kicks = {{
    {{{1, 0}, {1, 1}, {0, -2}, {1, -2}}},
    {{{1, 0}, {1, -1}, {0, 2}, {1, 2}}},
    {{{-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}},
    {{{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},
}};

static constexpr Kicks clockwise_I_kicks = {{
    {{{-2, 0}, {1, 0}, {1, 2}, {-2, -1}}},
    {{{-1, 0}, {2, 0}, {-1, 2}, {2, -1}}},
    {{{2, 0}, {-1, 0}, {2, 1}, {-1, -2}}},
    {{{-2, 0}, {1, 0}, {-2, 1}, {1, -2}}},
}};

static constexpr Kicks counter_clockwise_I_kicks = {{
    {{{2, 0}, {-1, 0}, {-1, 2}, {2, -1}}},
    {{{2, 0}, {-1, 0}, {2, 1}, {-1, -2}}},
    {{{-2, 0}, {1, 0}, {-2, 1}, {1, -2}}},
    {{{1, 0}, {-2, 0}, {1, 2}, {-2, -1}}},
}};
