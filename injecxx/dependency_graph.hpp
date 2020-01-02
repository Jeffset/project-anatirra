#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include "meta_constructor.hpp"
#include "type_array.hpp"

namespace base::injecxx::detail {

template <class T>
constexpr bool is_lazy(meta::ta<lazy<T>>) {
  return true;
}

template <class T>
constexpr bool is_lazy(meta::ta<T>) {
  return false;
}

template <class T>
constexpr auto get_type(meta::ta<T> type) {
  if constexpr (is_lazy(type)) {
    return meta::t<typename T::type>;
  } else {
    return type;
  }
}

template <typename... Leaves>
class dependency_graph {
 public:
  template <class... Ts>
  struct CyclicDependency {
    constexpr CyclicDependency(meta::ta<Ts...>) {}
    constexpr operator bool() { return false; }
  };

  template <class... Ts>
  CyclicDependency(meta::ta<Ts...>)->CyclicDependency<Ts...>;

  template <class... Ts>
  struct MissingDependency {
    constexpr MissingDependency(meta::ta<Ts...>) {}
    constexpr operator bool() { return false; }
  };

  template <class... Ts>
  MissingDependency(meta::ta<Ts...>)->MissingDependency<Ts...>;

  static constexpr auto all_types =
      meta::filter(meta::ts<Leaves...>, [](auto type) {
        constexpr auto all = meta::ts<Leaves...>;
        constexpr auto deps =
            meta::deduce_constructor_arg_types(type, all - type);
        return deps != meta::error_ta;
      });

  template <class T>
  static constexpr auto dependencies(meta::ta<T> type) {
    if constexpr (type == meta::null_ta) {
      return meta::empty_ta;
    } else {
      constexpr auto deps = meta::deduce_constructor_arg_types(
          get_type(type), all_types - meta::t<T>);
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
        static_assert(MissingDependency(element + deps),
                      "Missing dependencies present");
      }
    });
  }

  static constexpr auto topologically_sorted_types() {
    return topological_sort_impl(meta::empty_ta, meta::empty_ta,
                                 meta::empty_ta);
  }

 private:
  template <class T>
  static constexpr auto new_stack_frame(meta::ta<T> type) {
    constexpr auto deps =
        meta::map(dependencies(type), [](auto t) { return get_type(t); });
    return meta::wrap(type + deps);
  }

  template <typename... StackFrames>
  static constexpr auto report_cyclic_dependency(
      meta::type_array<StackFrames...> stack_frames) {
    constexpr auto deps_cycle =
        meta::map(meta::reverse(stack_frames), [](auto stack_frame) {
          return meta::decompose_first(meta::unwrap(stack_frame));
        });
    static_assert(CyclicDependency(deps_cycle),
                  "Cyclic dependency is detected");
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
        return topological_sort_impl<true>(meta::decompose_tail(stack), visited,
                                           closed);
      } else {
        if constexpr (!is_return && meta::contains(visited, current)) {
          return report_cyclic_dependency(stack);
        } else {
          constexpr auto new_visited = visited + current;
          if constexpr (unvisited_children == meta::empty_ta) {
            // no more children, mark current as closed and return.
            constexpr auto new_closed = closed + current;
            return topological_sort_impl<true>(meta::decompose_tail(stack),
                                               new_visited, new_closed) +
                   current;
          } else {
            constexpr auto new_current =
                meta::decompose_first(unvisited_children);
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

}  // namespace base::injecxx::detail

#endif  // DEPENDENCY_GRAPH_HPP
