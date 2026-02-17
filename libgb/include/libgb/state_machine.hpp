#pragma once

#include <libgb/format.hpp>
#include <libgb/std/meta.hpp>
#include <libgb/std/storage.hpp>
#include <libgb/std/traits.hpp>
#include <libgb/std/type_name.hpp>

namespace libgb {
template <typename T>
concept is_state = requires(T t) {
  { typename T::Connections{} } -> is_type_list;
  // TODO: work out a more sensible interface for the on_* functions
};

namespace impl {
template <is_state... VisitedStates, is_state... QueuedStates>
consteval auto calculate_possible_states(TypeList<VisitedStates...>,
                                         TypeList<QueuedStates...>) {
  using Visited = TypeList<VisitedStates...>;
  using Queued = TypeList<QueuedStates...>;

  if constexpr (sizeof...(QueuedStates) > 0) {
    using ThisState = meta::head<Queued>;
    if constexpr (meta::is_contained<ThisState, Visited>) {
      // We've already visited this state, drop it
      return calculate_possible_states(Visited{}, meta::tail<Queued>{});
    } else {
      // We've not visited this state, examine it
      return calculate_possible_states(
          meta::append<Visited, ThisState>{},
          meta::concat<Queued, typename ThisState::Connections>{});
    }
  } else {
    return Visited{};
  }
}

template <is_state From, is_state... AllStates, is_state... ToStates>
[[gnu::always_inline]] auto dump_state_edges(TypeList<AllStates...>,
                                             TypeList<ToStates...>) -> void {
  constexpr auto this_index = meta::find_index<TypeList<AllStates...>, From>;
  libgb::print<"S{#} [label=\"{}\"];">(this_index, type_name<From>());
  (libgb::print<"S{#} -> S{#};">(
       this_index, meta::find_index<TypeList<AllStates...>, ToStates>),
   ...);
}

template <is_state... States>
[[gnu::always_inline]] auto dump_state_names(TypeList<States...>) -> void {
  size_t index = 0;
  (libgb::print<"S{#} [label=\"{}\"];">(index++, type_name<States>()), ...);
}

template <is_state Entry, is_state... States>
[[gnu::always_inline]] auto dump_states_as_graph(TypeList<States...> all_states)
    -> void {
  libgb::print<"digraph {{">();
  dump_state_names(all_states);
  libgb::print<"Entry -> S{#};">(meta::find_index<TypeList<States...>, Entry>);
  (dump_state_edges<States>(all_states, typename States::Connections{}), ...);
  libgb::println<"}">();
}

template <is_state EntryPoint>
using AllStatesFrom = decltype(impl::calculate_possible_states(
    TypeList<>{}, TypeList<EntryPoint>{}));
} // namespace impl

/*
 * A partially type-erased StateMachine implementation.
 *
 * All transitions are strongly-typed, but dispatch is dynamic.
 * Impurity is supported: there is both global persistent state and internal
 * transient state. Adding additional pure states is cheaper than adding mutable
 * internal state.
 */
template <is_state EntryState, typename Storage = EmptyStorage,
          template <typename Stored> typename StorageType = Dynamic>
class StateMachine {
public:
  struct [[nodiscard]] Token {
    friend StateMachine;

  private:
    Token() = default;

  public:
    Token(Token &) = delete;
    Token(Token &&) = default;
    auto operator=(Token &&) -> Token & = delete;
    auto operator=(Token &) -> Token & = delete;
  };
  auto get_token() -> Token { return {}; }

private:
  using PossibleStates = impl::AllStatesFrom<EntryState>;
  using TransientStorage =
      meta::expand_into<libgb::MaybeEmptyStorageForAny, PossibleStates>;

  using ErasedOnTickFn = Token (*)(StateMachine &);

  struct InternalState {
    [[no_unique_address]] TransientStorage transient_storage;
    [[no_unique_address]] Storage non_transient_storage;
    ErasedOnTickFn on_tick_fn;
  };
  [[no_unique_address]] StorageType<InternalState> m_state = {};

  // In its own function to make the symbol names more reasonable
  template <meta::is_contained<PossibleStates> To>
  [[gnu::always_inline]] auto set_tick_fn() {
    m_state->on_tick_fn = [](StateMachine &state_machine) -> Token {
      if constexpr (is_statefull<To>) {
        auto &to = state_machine.m_state->transient_storage.template ref<To>();
        return to.on_tick(state_machine);
      } else {
        return To{}.on_tick(state_machine);
      }
    };
  }

  template <meta::is_contained<PossibleStates> To, typename... ToArgs>
  [[gnu::always_inline]] auto enter_state(ToArgs... args) {
    if constexpr (is_statefull<To>) {
      auto &to = m_state->transient_storage.template emplace<To>(
          libgb::forward<ToArgs>(args)...);
      to.on_entry(m_state->non_transient_storage);
    } else {
      To{}.on_entry(m_state->non_transient_storage);
    }
    set_tick_fn<To>();
  }

public:
  template <typename... StateArgs>
  explicit StateMachine(Storage storage, StateArgs... args) {
    m_state->non_transient_storage = storage;
    enter_state<EntryState>(libgb::forward<StateArgs>(args)...);
  }

  /* Dump the state transition diagram directly to serial */
  [[gnu::noinline]] static auto dump() -> void {
    impl::dump_states_as_graph<EntryState>(PossibleStates{});
  }

  template <meta::is_contained<PossibleStates> From,
            meta::is_contained<PossibleStates> To, typename... ToArgs>
  auto transition(ToArgs... args) -> void {
    if constexpr (is_statefull<From>) {
      auto &from = m_state->transient_storage.template ref<From>();
      from.on_exit(m_state->non_transient_storage);
      m_state->transient_storage.template destruct<From>();
    } else {
      From{}.on_exit(m_state->non_transient_storage);
    }

    enter_state<To>(libgb::forward<ToArgs>(args)...);
  }

  auto do_tick() -> void { (void)m_state->on_tick_fn(*this); }

  auto get_storage() -> Storage & { return m_state->non_transient_storage; };
  auto get_storage() const -> Storage const & {
    return m_state->non_transient_storage;
  };
};

template <is_state EntryState, typename Storage = EmptyStorage>
using StateMachineSingleton = StateMachine<EntryState, Storage, Singleton>;

template <typename Derived> struct StateBase {
  template <typename Storage> auto on_entry(Storage &) -> void {}
  template <typename Storage> auto on_exit(Storage &) -> void {}

  // MUST only be called as the last action in on_tick
  template <typename Other, typename StateMachine, typename... ToArgs>
  static auto transition_to(StateMachine &state_machine, ToArgs... args)
      -> StateMachine::Token {
    static_assert(
        meta::is_contained<Other, typename Derived::Connections>,
        "Attempting a state transition that is not in the Connections list");
    state_machine.template transition</*From=*/Derived, /*To=*/Other>(
        libgb::forward<ToArgs>(args)...);
    return state_machine.get_token();
  }

  template <typename StateMachine>
  static auto remain(StateMachine &state_machine) -> StateMachine::Token {
    return state_machine.get_token();
  }
};
} // namespace libgb
