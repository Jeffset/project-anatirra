// Created by jeffset on 12/23/19.

#ifndef INJECXX_INJECXX_CONTEXT_HPP
#define INJECXX_INJECXX_CONTEXT_HPP

#include "dependency_graph.hpp"
#include "injecxx.hpp"

#include <utility>

#define REQUIRES(condition) typename = std::enable_if_t<(condition), void>

namespace base::injecxx {

namespace detail {

template <class T>
struct instance_holder {
  bool initialized : 1;
  unsigned char data[sizeof(T)];

  instance_holder() noexcept : initialized(false) {}
};

struct MissingDependency {};

template <class T>
constexpr bool is_plain_v = std::is_same_v<T, std::decay_t<T>>;

}  // namespace detail

template <typename... Ts>
class context_impl : private injectable,
                     private detail::instance_holder<Ts>... {
  static_assert((!std::is_copy_constructible_v<Ts> && ...) &&
                    (!std::is_copy_assignable_v<Ts> && ...) &&
                    (!std::is_move_assignable_v<Ts> && ...) &&
                    (!std::is_move_constructible_v<Ts> && ...),
                "All injectable components must not be copy/move "
                "constructible/assignable");

 public:
  static constexpr auto all_types() { return meta::ts<Ts...>; }

 public:
  context_impl() noexcept = default;

  ~context_impl() noexcept {
    constexpr auto graph = detail::make_graph(all_types());
    constexpr auto sorted_types =
        meta::filter_not_null(graph.topologically_sorted_types());
    meta::for_each(sorted_types, [this](auto type) {
      using T = meta::ta_single_t<decltype(type)>;
      if constexpr (type != meta::null_ta) {
        using provider_t = detail::instance_holder<T>;
        if (provider_t::initialized) {
          auto* data = reinterpret_cast<T*>(provider_t::data);
          data->~T();
        }
      }
      (void)this;  // |this| is actually used above, but clang
                   // still emits a warning, so suppress it.
    });
  }

  template <class T, class DelegatedContext, REQUIRES(detail::is_plain_v<T>)>
  decltype(auto) get(DelegatedContext& context) noexcept {
    constexpr auto requested_type = meta::t<T>;
    constexpr auto type = detail::get_type(requested_type);
    constexpr auto delegated_all_types = DelegatedContext::all_types();
    if constexpr (meta::contains(delegated_all_types, type)) {
      if constexpr (detail::is_lazy(requested_type)) {
        return lazy_get(context, type);
      } else {
        constexpr auto graph = detail::make_graph(delegated_all_types);
        return construct<T>(context, graph.dependencies(type));
      }
    } else {
      return detail::MissingDependency{};
    }
  }

  template <class DelegatedContext, class Filter, class Dispatcher>
  void dispatch(DelegatedContext& context,
                Filter filter,
                const Dispatcher& dispatcher) {
    constexpr auto types = meta::filter(all_types(), filter);
    static_assert(types != meta::empty_ta, "No types would pass the filter");
    meta::for_each(types, [&context, &dispatcher](auto type) {
      using T = meta::ta_single_t<decltype(type)>;
      T& instance = context.template get<T>();
      if constexpr (std::is_invocable_v<decltype(dispatcher), T&,
                                        DelegatedContext&>) {
        dispatcher(instance, context);
      } else {
        static_assert(std::is_invocable_v<decltype(dispatcher), T&>);
        dispatcher(instance);
      }
    });
  }

  template <class DelegatedContext, class Dispatcher>
  void dispatch(DelegatedContext& context, const Dispatcher& dispatcher) {
    constexpr auto default_filter = [](auto t) {
      using T = meta::ta_single_t<decltype(t)>;
      return std::is_invocable_v<Dispatcher, T&> or
             std::is_invocable_v<Dispatcher, T&, DelegatedContext&>;
    };
    dispatch(context, default_filter, dispatcher);
  }

 private:
  template <class T, class DelegatedContext, class... Args>
  T& construct(DelegatedContext& context, meta::ta<Args...> args) noexcept {
    using provider_t = detail::instance_holder<T>;
    if (!provider_t::initialized) {
      if constexpr (!meta::contains(args, meta::null_ta)) {
        static_assert(std::is_nothrow_constructible_v<T, Args&...>,
                      "Component must be noexcept constructible");
      }
      new (provider_t::data) T(context.template get<Args>()...);
      provider_t::initialized = true;
    }
    return *reinterpret_cast<T*>(provider_t::data);
  }

  template <class DelegatedContext, class T>
  lazy<T> lazy_get(DelegatedContext& context, meta::ta<T>) noexcept {
    return lazy<T>{&context, &DelegatedContext::template get<T>};
  }
};

template <class ParentContext, class... Ts>
class context_wrapper {
  using ChildContext = context_impl<Ts...>;

  static_assert(!meta::contains(ParentContext::all_types(),
                                ChildContext::all_types()),
                "Contexts must not contain duplicate classes");

 public:
  constexpr explicit context_wrapper(ParentContext& parent_context) noexcept
      : parent_context_(parent_context) {
    detail::make_graph(all_types()).check_for_missing_deps();
  }

  static constexpr auto all_types() {
    return ChildContext::all_types() + ParentContext::all_types();
  }

  template <class T>
  decltype(auto) get() noexcept {
    if constexpr (meta::contains(ParentContext::all_types(), meta::t<T>)) {
      return parent_context_.template get<T>();
    } else {
      return context_.template get<T>(*this);
    }
  }

  template <class Filter, class Dispatcher>
  void dispatch(Filter filter, const Dispatcher& dispatcher) {
    // No parent context dispatch.
    context_.dispatch(*this, filter, dispatcher);
  }

  template <class Dispatcher>
  void dispatch(const Dispatcher& dispatcher) {
    // No parent context dispatch.
    context_.dispatch(*this, dispatcher);
  }

 private:
  ParentContext& parent_context_;
  context_impl<Ts...> context_;
};

template <class... Ts>
class context_wrapper<void, Ts...> {
  using Context = context_impl<Ts...>;

 public:
  constexpr context_wrapper() noexcept {
    detail::make_graph(all_types()).check_for_missing_deps();
  }

  static constexpr auto all_types() { return Context::all_types(); }

  template <class T>
  decltype(auto) get() noexcept {
    return context_.template get<T>(*this);
  }

  template <class Filter, class Dispatcher>
  void dispatch(Filter filter, const Dispatcher& dispatcher) {
    context_.dispatch(*this, filter, dispatcher);
  }

  template <class Dispatcher>
  void dispatch(const Dispatcher& dispatcher) {
    context_.dispatch(*this, dispatcher);
  }

 private:
  context_impl<Ts...> context_;
};

template <class... Ts>
auto make_context() {
  return context_wrapper<void, Ts...>{};
}

template <class... Ts, class ParentContext>
auto make_context(ParentContext& parent) {
  return context_wrapper<ParentContext, Ts...>{parent};
}

}  // namespace base::injecxx

#endif  // INJECXX_INJECXX_CONTEXT_HPP
