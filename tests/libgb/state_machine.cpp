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

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token {
    m_frame += 1;
    sm.get_storage().idle_count += 1;

    if (m_frame == 3) {
      return this->transition_to<Walking>(sm);
    }

    // Unreachable but interesting to test the Connection resolution logic
    if (m_frame == 4) {
      return transition_to<Running>(sm);
    }

    return remain(sm);
  }
};
struct Walking : libgb::StateBase<Walking> {
  using Connections = libgb::TypeList<Running>;

  uint8_t m_frame = 0;

  template <typename StateMachine>
  auto on_tick(StateMachine &sm) -> StateMachine::Token {
    m_frame += 1;
    sm.get_storage().walking_count += 1;

    if (m_frame == 2) {
      return transition_to<Running>(sm);
    }
    return remain(sm);
  }
};

struct Running : libgb::StateBase<Running> {
  using Connections = libgb::TypeList<Idle>;

  template <typename StateMachine>
  static auto on_tick(StateMachine &sm) -> StateMachine::Token {
    sm.get_storage().running_count += 1;
    return Running::transition_to<Idle>(sm);
  }
};
} // namespace

static libgb::StateMachine<Idle, Storage> impure_machine{{}};
static libgb::StateMachineSingleton<Idle, Storage> singleton{{}};

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

  template <typename StateMachine>
  static auto on_tick(StateMachine &sm) -> StateMachine::Token {
    return remain(sm);
  }
};

struct S2 : libgb::StateBase<S1> {
  using Connections = libgb::TypeList<S1, S3>;

  template <typename StateMachine>
  static auto on_tick(StateMachine &sm) -> StateMachine::Token {
    return remain(sm);
  }
};

struct S3 : libgb::StateBase<S3> {
  using Connections = libgb::TypeList<S1>;

  template <typename StateMachine>
  static auto on_tick(StateMachine &sm) -> StateMachine::Token {
    return remain(sm);
  }
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

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0001, walking_count=$0000, running_count=$0000

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0002, walking_count=$0000, running_count=$0000

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0000, running_count=$0000

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0001, running_count=$0000

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0002, running_count=$0000

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0003, walking_count=$0002, running_count=$0001

  singleton.do_tick();
  print_state(singleton.get_storage());
  // CHECK: idle_count=$0004, walking_count=$0002, running_count=$0001

  singleton.dump();
  // CHECK: digraph {
  // CHECK: S0000 [label="Idle"];
  // CHECK: S0001 [label="Walking"];
  // CHECK: S0002 [label="Running"];
  // CHECK: Entry -> S0000;
  // CHECK: S0000 -> S0001;
  // CHECK: S0000 -> S0002;
  // CHECK: S0001 -> S0002;
  // CHECK: S0002 -> S0000;
  // CHECK: }
  return 0;
}
