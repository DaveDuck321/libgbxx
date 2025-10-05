// RUN: $GAMEBOY_EMULATOR_PATH/emulate.out $GBLIB_BUILD_DIR/state_machine.out \
// RUN:   | FileCheck %s -check-prefix=CHECK

#include <libgb/format.hpp>
#include <libgb/interrupts.hpp>
#include <libgb/state_machine.hpp>
#include <libgb/std/meta.hpp>

namespace {
struct Storage {
  uint8_t idle_count = 0;
  uint8_t walking_count = 0;
  uint8_t running_count = 0;
};

struct Idle;
struct Walking;
struct Running;

struct Idle : libgb::StateBase<Idle> {
  using Connections = libgb::TypeList<Walking, Running>;

  uint8_t m_frame = 0;

  template <typename Storage> auto on_entry(Storage &) -> void {}
  template <typename Storage> auto on_exit(Storage &) -> void {}
  template <typename StateMachine> auto on_tick(StateMachine &sm) -> void {
    m_frame += 1;
    sm.get_storage().idle_count += 1;

    if (m_frame == 3) {
      this->transition_to<Walking>(sm);
    }

    // Unreachable but interesting to test the Connection resolution logic
    if (m_frame == 4) {
      this->transition_to<Running>(sm);
    }
  }
};
struct Walking : libgb::StateBase<Walking> {
  using Connections = libgb::TypeList<Running>;

  uint8_t m_frame = 0;

  template <typename Storage> auto on_entry(Storage &) -> void {}
  template <typename Storage> auto on_exit(Storage &) -> void {}
  template <typename StateMachine> auto on_tick(StateMachine &sm) -> void {
    m_frame += 1;
    sm.get_storage().walking_count += 1;

    if (m_frame == 2) {
      this->transition_to<Running>(sm);
    }
  }
};

struct Running : libgb::StateBase<Running> {
  using Connections = libgb::TypeList<Idle>;

  template <typename Storage> static auto on_entry(Storage &) -> void {}
  template <typename Storage> static auto on_exit(Storage &) -> void {}
  template <typename StateMachine>
  static auto on_tick(StateMachine &sm) -> void {
    sm.get_storage().running_count += 1;
    Running::transition_to<Idle>(sm);
  }
};
} // namespace

static libgb::StateMachine<Idle, Storage> impure_machine{{}};

static auto print_state(Storage &get_storage) -> void {
  libgb::println<"idle_count={}, walking_count={}, running_count={}">(
      get_storage.idle_count, get_storage.walking_count,
      get_storage.running_count);
}

struct S1;
struct S2;
struct S3;

struct S1 : libgb::StateBase<S1> {
  using Connections = libgb::TypeList<S2, S3>;

  template <typename Storage> static auto on_entry(Storage &) -> void {}
  template <typename Storage> static auto on_exit(Storage &) -> void {}
  template <typename StateMachine>
  static auto on_tick(StateMachine &) -> void {}
};

struct S2 : libgb::StateBase<S1> {
  using Connections = libgb::TypeList<S1, S3>;

  template <typename Storage> static auto on_entry(Storage &) -> void {}
  template <typename Storage> static auto on_exit(Storage &) -> void {}
  template <typename StateMachine>
  static auto on_tick(StateMachine &) -> void {}
};

struct S3 : libgb::StateBase<S3> {
  using Connections = libgb::TypeList<S1>;

  template <typename Storage> static auto on_entry(Storage &) -> void {}
  template <typename Storage> static auto on_exit(Storage &) -> void {}
  template <typename StateMachine>
  static auto on_tick(StateMachine &) -> void {}
};

// A pure state machine only needs to store the pointer to on_tick_fn
static_assert(sizeof(libgb::StateMachine<S1>) == sizeof(void *));

int main() {
  libgb::enable_interrupts();

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0001, walking_count=$0000, running_count=$0000

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0002, walking_count=$0000, running_count=$0000

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0000, running_count=$0000

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0001, running_count=$0000

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0002, running_count=$0000

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0002, running_count=$0001

  impure_machine.do_tick();
  print_state(impure_machine.get_storage());
  // CHECK: idle_count=$0004, walking_count=$0002, running_count=$0001
  return 0;
}
