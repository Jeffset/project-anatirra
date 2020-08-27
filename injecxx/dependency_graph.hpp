// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_INJECXX_DEPENDENCY_GRAPH
#define ANATIRRA_INJECXX_DEPENDENCY_GRAPH

#include "base/type_array.hpp"
#include "injecxx/injecxx.hpp"
#include "injecxx/meta_constructor.hpp"

namespace injecxx::detail {

using namespace base;

template <class T>
constexpr bool is_lazy(meta::ta<lazy<T>>) {
  return true;
}

template <class T>
constexpr bool is_lazy(meta::ta<T>) {
  return false;
}

template <class T>
constexpr auto get_dependency_type(meta::ta<T> type) {
  if constexpr (is_lazy(type)) {
    return meta::t<typename T::type>;
  } else {
    return type;
  }
}

template <class T>
constexpr auto get_context_type(meta::ta<T> type) {
  static_assert(!is_lazy(type),
                "Improper use of lazy<> decorator: use it in component's constructors,"
                "not in a context types list.");
  return type;
}

template <class... Ts>
struct CyclicDependency {
  constexpr CyclicDependency(meta::ta<Ts...>) {}
  constexpr operator bool() { return false; }
};

template <class... Ts>
CyclicDependency(meta::ta<Ts...>) -> CyclicDependency<Ts...>;

template <class... Ts>
struct MissingDependencyMark {
  constexpr MissingDependencyMark(meta::ta<Ts...>) {}
  constexpr operator bool() { return false; }
};

template <class... Ts>
MissingDependencyMark(meta::ta<Ts...>) -> MissingDependencyMark<Ts...>;

template <typename... Leaves>
class dependency_graph {
 public:
  static constexpr auto all_types = meta::filter(meta::ts<Leaves...>, [](auto type) {
    constexpr auto all_provided =
        meta::map(meta::ts<Leaves...>, [](auto t) { return get_context_type(t); });
    constexpr auto deps = deduce_constructor_arg_types(type, all_provided - type);
    return deps != meta::error_ta;
  });

  template <class T>
  static constexpr auto dependencies(meta::ta<T> type) {
    if constexpr (type == meta::null_ta) {
      return meta::empty_ta;
    } else {
      constexpr auto all_provided_deps =
          meta::map(all_types - type, [](auto t) { return get_context_type(t); });
      constexpr auto deps =
          deduce_constructor_arg_types(get_dependency_type(type), all_provided_deps);
      if constexpr (deps == meta::error_ta) {
        return meta::empty_ta;
      } else {
        return deps;
      }
    }
  }

  static constexpr void check_for_missing_deps() {
    meta::for_each(all_types, [](auto element) {
      constexpr auto deps = dependencies(element);
      constexpr bool missing_deps_present = meta::contains(deps, meta::null_ta);
      if constexpr (missing_deps_present) {
        static_assert(MissingDependencyMark(element + deps),
                      "Missing dependencies present");
      }
    });
  }

  static constexpr auto topologically_sorted_types() {
    return topological_sort_impl(meta::empty_ta, meta::empty_ta, meta::empty_ta);
  }

 private:
  template <class T>
  static constexpr auto new_stack_frame(meta::ta<T> type) {
    constexpr auto deps =
        meta::map(dependencies(type), [](auto t) { return get_dependency_type(t); });
    return meta::wrap(type + deps);
  }

  template <typename... StackFrames>
  static constexpr auto report_cyclic_dependency(
      meta::type_array<StackFrames...> stack_frames) {
    constexpr auto deps_cycle =
        meta::map(meta::reverse(stack_frames), [](auto stack_frame) {
          return meta::decompose_first(meta::unwrap(stack_frame));
        });
    static_assert(CyclicDependency(deps_cycle), "Cyclic dependency is detected");
    return meta::empty_ta;
  }

  template <bool is_return = false,
            typename... Stack,
            typename... Visited,
            typename... Closed>
  static constexpr auto topological_sort_impl(meta::ta<Stack...> stack,
                                              meta::ta<Visited...> visited,
                                              meta::ta<Closed...> closed) {
    // stack maybe empty - use unvisited.
    if constexpr (stack == meta::empty_ta) {
      constexpr auto unvisited = all_types - visited;
      if constexpr (unvisited == meta::empty_ta) {
        // end of algorithm.
        return meta::empty_ta;
      } else {
        constexpr auto new_current = meta::decompose_first(unvisited);
        constexpr auto new_stack = new_stack_frame(new_current);
        return topological_sort_impl(new_stack, visited, closed);
      }
    } else {
      constexpr auto current_stack = meta::unwrap(meta::decompose_first(stack));
      constexpr auto current = meta::decompose_first(current_stack);
      constexpr auto unvisited_children = meta::decompose_tail(current_stack);
      if constexpr (meta::contains(closed, current)) {
        // current is marked as closed - simply return.
        return topological_sort_impl<true>(meta::decompose_tail(stack), visited, closed);
      } else {
        if constexpr (!is_return && meta::contains(visited, current)) {
          return report_cyclic_dependency(stack);
        } else {
          constexpr auto new_visited = visited + current;
          if constexpr (unvisited_children == meta::empty_ta) {
            // no more children, mark current as closed and return.
            constexpr auto new_closed = closed + current;
            return topological_sort_impl<true>(meta::decompose_tail(stack), new_visited,
                                               new_closed) +
                   current;
          } else {
            constexpr auto new_current = meta::decompose_first(unvisited_children);
            constexpr auto new_stack =
                new_stack_frame(new_current) +
                meta::wrap(current + meta::decompose_tail(unvisited_children)) +
                meta::decompose_tail(stack);
            return topological_sort_impl(new_stack, new_visited, closed);
          }
        }
      }
    }
  }
};

template <class... Ts>
constexpr auto make_graph(meta::ta<Ts...>) {
  return dependency_graph<Ts...>{};
}

}  // namespace injecxx::detail

#endif  // ANATIRRA_INJECXX_DEPENDENCY_GRAPH
