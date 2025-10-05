#pragma once

#include <libgb/std/meta.hpp>
#include <libgb/std/storage.hpp>
#include <libgb/std/traits.hpp>

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
template <is_state EntryState, typename Storage = EmptyStorage>
class StateMachine {
  using PossibleStates = impl::AllStatesFrom<EntryState>;
  using TransientStorage =
      meta::expand_into<libgb::MaybeEmptyStorageForAny, PossibleStates>;

  using ErasedOnTickFn = void (*)(StateMachine &);

  [[no_unique_address]] TransientStorage transient_storage;
  [[no_unique_address]] Storage non_transient_storage;
  ErasedOnTickFn on_tick_fn;

  // In its own function to make the symbol names more reasonable
  template <meta::is_contained<PossibleStates> To>
  [[gnu::always_inline]] auto set_tick_fn() {
    on_tick_fn = [](StateMachine &state_machine) {
      if constexpr (is_statefull<To>) {
        auto &to = state_machine.transient_storage.template ref<To>();
        to.on_tick(state_machine);
      } else {
        To{}.on_tick(state_machine);
      }
    };
  }

  template <meta::is_contained<PossibleStates> To, typename... ToArgs>
  [[gnu::always_inline]] auto enter_state(ToArgs... args) {
    if constexpr (is_statefull<To>) {
      auto &to = transient_storage.template emplace<To>(
          libgb::forward<ToArgs>(args)...);
      to.on_entry(non_transient_storage);
    } else {
      To{}.on_entry(non_transient_storage);
    }
    set_tick_fn<To>();
  }

public:
  template <typename... StateArgs>
  explicit StateMachine(Storage storage, StateArgs... args)
      : non_transient_storage(storage) {
    enter_state<EntryState>(libgb::forward<StateArgs>(args)...);
  }

  template <meta::is_contained<PossibleStates> From,
            meta::is_contained<PossibleStates> To, typename... ToArgs>
  auto transition(ToArgs... args) -> void {
    if constexpr (is_statefull<To>) {
      auto &from = transient_storage.template ref<From>();
      from.on_exit(non_transient_storage);
      transient_storage.template destruct<From>();
    } else {
      From{}.on_exit(non_transient_storage);
    }

    enter_state<To>(libgb::forward<ToArgs>(args)...);
  }

  auto do_tick() -> void { on_tick_fn(*this); }

  auto get_storage() -> Storage & { return non_transient_storage; };
  auto get_storage() const -> Storage const & { return non_transient_storage; };
};

template <typename Derived> struct StateBase {
  // MUST only be called as the last action in on_tick
  template <typename Other, typename StateMachine, typename... ToArgs>
  static auto transition_to(StateMachine &state_machine, ToArgs... args)
      -> void {
    static_assert(
        meta::is_contained<Other, typename Derived::Connections>,
        "Attempting a state transition that is not in the Connections list");
    state_machine.template transition</*From=*/Derived, /*To=*/Other>(
        libgb::forward<ToArgs>(args)...);
  }
};
} // namespace libgb
