// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_INJECXX_INJECXX_CONTEXT
#define ANATIRRA_INJECXX_INJECXX_CONTEXT

#include "base/macro.hpp"
#include "injecxx/dependency_graph.hpp"
#include "injecxx/injecxx.hpp"

#include <optional>
#include <utility>

namespace injecxx {

using namespace base;

namespace detail {

template <class T>
using instance_holder = std::optional<T>;

struct MissingDependency {};

template <class T>
constexpr bool is_plain_v = std::is_same_v<T, std::decay_t<T>>;

}  // namespace detail

template <typename... Ts>
class context_impl : private detail::instance_holder<Ts>... {
 public:
  MAKE_FULLY_STATIONAR(context_impl);

  constexpr inline context_impl() noexcept {
    constexpr auto all_unwrapped_types =
        meta::map(meta::ts<Ts...>, [](auto t) { return detail::get_context_type(t); });

    meta::for_each(all_unwrapped_types, [](auto t) {
      using T = meta::ta_single_t<decltype(t)>;
      static_assert(!std::is_copy_constructible_v<T> && !std::is_copy_assignable_v<T> &&
                        !std::is_move_assignable_v<T> && !std::is_move_constructible_v<T>,
                    "All injectable components must not be copy/move "
                    "constructible/assignable");
    });
  }

 public:
  static constexpr auto all_types() { return meta::ts<Ts...>; }

  ~context_impl() noexcept {
    constexpr auto graph = detail::make_graph(all_types());
    constexpr auto sorted_types =
        meta::filter_not_null(graph.topologically_sorted_types());
    meta::for_each(sorted_types, [this](auto type) {
      using T = meta::ta_single_t<decltype(type)>;
      if constexpr (type != meta::null_ta) {
        using provider_t = detail::instance_holder<T>;
        provider_t::reset();
      }
      MARK_UNUSED(this);  // |this| is actually used above, but clang
                          // still emits a warning, so suppress it.
    });
  }

  template <class T, class DelegatedContext, REQUIRES(detail::is_plain_v<T>)>
  decltype(auto) get(DelegatedContext& context) noexcept {
    constexpr auto requested_type = meta::t<T>;
    constexpr auto type = detail::get_dependency_type(requested_type);
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
  void dispatch(DelegatedContext& context, Filter filter, const Dispatcher& dispatcher) {
    constexpr auto types = meta::filter(all_types(), filter);
    static_assert(types != meta::empty_ta, "No types would pass the filter");
    meta::for_each(types, [&context, &dispatcher](auto type) {
      using T = meta::ta_single_t<decltype(type)>;
      T& instance = context.template get<T>();
      if constexpr (std::is_invocable_v<decltype(dispatcher), T&, DelegatedContext&>) {
        dispatcher(instance, context);
      } else {
        static_assert(std::is_invocable_v<decltype(dispatcher), T&>);
        dispatcher(instance);
      }
    });
  }

 private:
  template <class T, class DelegatedContext, class... Args>
  T& construct(DelegatedContext& context, meta::ta<Args...> args) noexcept {
    using provider_t = detail::instance_holder<T>;
    if (!provider_t::has_value()) {
      if constexpr (!meta::contains(args, meta::null_ta)) {
        static_assert(std::is_nothrow_constructible_v<T, Args&...>,
                      "Component must be noexcept constructible");
      }
      provider_t::emplace(context.template get<Args>()...);
    }
    return provider_t::value();
  }

  template <class DelegatedContext, class T>
  lazy<T> lazy_get(DelegatedContext& context, meta::ta<T>) noexcept {
    return lazy<T>{&context, &DelegatedContext::template get<T>};
  }
};

template <class ParentContext, class... Ts>
class context_wrapper {
  using ChildContext = context_impl<Ts...>;

  static_assert(!meta::contains(ParentContext::all_types(), ChildContext::all_types()),
                "Contexts must not contain duplicate classes");

 public:
  constexpr explicit context_wrapper(ParentContext& parent_context) noexcept
      : parent_context_(parent_context) {
    detail::make_graph(all_types()).check_for_missing_deps();
  }

 private:
  template <class, class...>
  friend class context_wrapper;
  template <typename...>
  friend class context_impl;

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

 public:
  template <class Filter, class Dispatcher>
  void dispatch(Filter filter, const Dispatcher& dispatcher) {
    // No parent context dispatch.
    context_.dispatch(*this, filter, dispatcher);
  }

  template <class Dispatcher>
  void dispatch(const Dispatcher& dispatcher) {
    // No parent context dispatch.
    constexpr auto default_filter = [](auto t) {
      using T = meta::ta_single_t<decltype(t)>;
      return std::is_invocable_v<Dispatcher, T&> or
             std::is_invocable_v<Dispatcher, T&, context_wrapper&>;
    };
    context_.dispatch(*this, default_filter, dispatcher);
  }

  MAKE_FULLY_STATIONAR(context_wrapper);

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

 private:
  template <class, class...>
  friend class context_wrapper;
  template <typename...>
  friend class context_impl;

  static constexpr auto all_types() { return Context::all_types(); }

  template <class T>
  decltype(auto) get() noexcept {
    return context_.template get<T>(*this);
  }

 public:
  template <class Filter, class Dispatcher>
  void dispatch(Filter filter, const Dispatcher& dispatcher) {
    context_.dispatch(*this, filter, dispatcher);
  }

  template <class Dispatcher>
  void dispatch(const Dispatcher& dispatcher) {
    constexpr auto default_filter = [](auto t) {
      using T = meta::ta_single_t<decltype(t)>;
      return std::is_invocable_v<Dispatcher, T&> or
             std::is_invocable_v<Dispatcher, T&, context_wrapper&>;
    };
    context_.dispatch(*this, default_filter, dispatcher);
  }

  MAKE_FULLY_STATIONAR(context_wrapper);

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

}  // namespace injecxx

#endif  // ANATIRRA_INJECXX_INJECXX_CONTEXT
