#pragma once

#include <libgb/state_machine.hpp>

namespace {

struct GameplayUpdate;
struct BoardReset;
struct LineClear;
struct LevelClear;
struct HidingStars;

enum class ResetType {
  level_clear,
  game_over,
};

struct GameplayStorage {
  uint8_t current_level = 0;
};

struct GameplayUpdate : libgb::StateBase<GameplayUpdate> {
  using Connections = libgb::TypeList<LineClear, LevelClear, HidingStars>;

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token;
};

struct BoardReset : libgb::StateBase<BoardReset> {
  using Connections = libgb::TypeList<GameplayUpdate>;

  explicit BoardReset(ResetType type);

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token;

  template <typename Storage> auto on_entry(Storage &) -> void;
  template <typename Storage> auto on_exit(Storage &) -> void;

  ResetType m_type;
  uint8_t m_current_row;
};

struct LineClear : libgb::StateBase<LineClear> {
  using Connections = libgb::TypeList<GameplayUpdate>;

  explicit LineClear(uint8_t lines_left_to_clear)
      : m_lines_left_to_clear{lines_left_to_clear} {}

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token;

  uint8_t m_lines_left_to_clear;
  uint8_t m_animation_frame = 0;
};

struct HidingStars : libgb::StateBase<HidingStars> {
  using Connections = libgb::TypeList<BoardReset>;

  explicit HidingStars(ResetType type) : m_type{type} {}

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token;

  ResetType m_type;
};

struct LevelClear : libgb::StateBase<LevelClear> {
  using Connections = libgb::TypeList<HidingStars>;

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token;

  uint8_t m_current_row = 0;
};

} // namespace
