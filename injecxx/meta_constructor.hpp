// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_SRC_INJECXX_META_CONSTRUCTOR
#define ANATIRRA_SRC_INJECXX_META_CONSTRUCTOR

#include "base/type_array.hpp"
#include "injecxx/injecxx.hpp"

#include <cmath>

namespace injecxx::detail {

using namespace base;

namespace detail {

constexpr int MAX_ALLOWED_ARGS = 32;

template <class T>
struct specific_matcher {
  operator T();
};

template <class Exclude, class... Ts>
struct matcher /*: public specific_matcher<Ts>...*/ {
  template <class T,
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Exclude>, T>>
  operator T&();
};

template <class Subject, class... Ms>
constexpr bool is_constructible(meta::ta<Subject>, meta::ta<Ms...> ms) {
  return ms != meta::empty_ta && std::is_constructible_v<Subject, Ms...>;
}

template <class Subject, class... Ms, class... TypesPool>
constexpr auto gen_constructor_arg_placeholders_impl(meta::ta<TypesPool...> types_pool,
                                                     meta::ta<Subject> subject,
                                                     meta::ta<Ms...> ms) {
  if constexpr (is_constructible(subject, ms)) {
    return ms;
  } else {
    if constexpr (size(ms) >= MAX_ALLOWED_ARGS) {
      return meta::empty_ta;
    } else {
      constexpr auto v1_branch = gen_constructor_arg_placeholders_impl(
          types_pool, subject, ms + meta::t<matcher<Subject, TypesPool...>>);
      return v1_branch;
    }
  }
}

template <class Subject, class... TypesPool>
constexpr auto gen_constructor_arg_placeholders(meta::ta<Subject> subject,
                                                meta::ta<TypesPool...> types_pool) {
  return gen_constructor_arg_placeholders_impl(types_pool, subject, meta::empty_ta);
}

template <class Subject, class... TypesPool, class... Args, class... Ms>
constexpr auto gen_constructor_arg_types(meta::ta<Subject> subject,
                                         meta::ta<TypesPool...> types_pool,
                                         meta::ta<Args...> deduced_args,
                                         meta::ta<Ms...> placeholders) {
  constexpr auto deduced = ([]() {
    if constexpr (std::is_constructible_v<Subject, Args&..., TypesPool&, Ms...>) {
      return meta::t<TypesPool>;
    } else if constexpr (std::is_constructible_v<Subject, Args&..., lazy<TypesPool>,
                                                 Ms...>) {
      return meta::t<lazy<TypesPool>>;
    } else {
      return meta::empty_ta;
    }
  }() + ... + meta::empty_ta);
  constexpr auto deduced_or_null = [deduced]() {
    if constexpr (deduced == meta::empty_ta) {
      return meta::null_ta;
    } else if constexpr (size(deduced) > 1) {
      return meta::null_ta;  // TODO: this is not missing dependency, this is ambiguous
                             // dependency - report it specificly.
    } else {
      return deduced;
    }
  }();
  constexpr auto new_deduced_args = deduced_args + deduced_or_null;
  if constexpr (placeholders == meta::empty_ta) {
    return new_deduced_args;
  } else {
    return gen_constructor_arg_types(subject, types_pool, new_deduced_args,
                                     decompose_tail(placeholders));
  }
}

}  // namespace detail

template <class Subject, class... TypesPool>
constexpr auto deduce_constructor_arg_types(meta::ta<Subject> subject,
                                            meta::ta<TypesPool...> types_pool) {
  if constexpr (std::is_default_constructible_v<Subject>) {
    return meta::empty_ta;
  } else {
    constexpr auto placeholders =
        detail::gen_constructor_arg_placeholders(subject, types_pool);
    if constexpr (placeholders == meta::empty_ta) {
      static_assert(placeholders != meta::empty_ta,
                    "Unable to detect any usable constructors. This may be "
                    "because there's no "
                    "accessible constructors at all, or constructor is declared "
                    "in some really unexpected way, or there is a constructor with more "
                    "than 32 arguments which is a little to much. Try to correct "
                    "these problems and compile again.");
      return meta::error_ta;
    } else {
      return detail::gen_constructor_arg_types(subject, types_pool, meta::empty_ta,
                                               decompose_tail(placeholders));
    }
  }
}

}  // namespace injecxx::detail

#endif  // ANATIRRA_SRC_INJECXX_META_CONSTRUCTOR
