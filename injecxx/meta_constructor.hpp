//
// Created by jeffset on 12/26/19.
//

#ifndef INJECXX_META_CONSTRUCTOR_HPP
#define INJECXX_META_CONSTRUCTOR_HPP

#include "injecxx.hpp"
#include "type_array.hpp"

namespace base::meta {

namespace detail {

template <class T>
struct specific_matcher {
  operator T();
};

template <class Exclude, class... Ts>
struct matcher : public specific_matcher<Ts>... {
  template <class T,
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Exclude>, T>>
  operator T&();
};

template <class Subject, class... Ms>
constexpr bool is_constructible(ta<Subject>, ta<Ms...> ms) {
  return ms != meta::empty_ta && std::is_constructible_v<Subject, Ms...>;
}

template <class Subject, class... Ms, class... TypesPool>
constexpr auto gen_constructor_arg_placeholders_impl(ta<TypesPool...> types_pool,
                                                     ta<Subject> subject,
                                                     ta<Ms...> ms) {
  if constexpr (is_constructible(subject, ms)) {
    return ms;
  } else {
    if constexpr (meta::size(ms) >= 32) {
      return empty_ta;
    } else {
      constexpr auto v1_branch = gen_constructor_arg_placeholders_impl(
          types_pool, subject, ms + t<matcher<Subject, TypesPool...>>);
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
constexpr auto gen_constructor_arg_types(ta<Subject> subject,
                                         ta<TypesPool...> types_pool,
                                         ta<Args...> deduced_args,
                                         ta<Ms...> placeholders) {
  // TODO: rewrite it from O(n^2) to O(n log n) using specific_multi_matcher
  constexpr auto deduced = ([]() {
    if constexpr (std::is_constructible_v<Subject, Args&..., TypesPool&, Ms...>) {
      return t<TypesPool>;
    } else if constexpr (std::is_constructible_v<Subject, Args&...,
                                                 injecxx::lazy<TypesPool>, Ms...>) {
      return t<injecxx::lazy<TypesPool>>;
    } else {
      return empty_ta;
    }
  }() + ... + empty_ta);
  constexpr auto deduced_or_null = [deduced]() {
    if constexpr (deduced == empty_ta || size(deduced) > 1) {
      return meta::null_ta;
    } else {
      return deduced;
    }
  }();
  constexpr auto new_deduced_args = deduced_args + deduced_or_null;
  if constexpr (placeholders == empty_ta) {
    return new_deduced_args;
  } else {
    return gen_constructor_arg_types(subject, types_pool, new_deduced_args,
                                     decompose_tail(placeholders));
  }
}

}  // namespace detail

template <class Subject, class... TypesPool>
constexpr auto deduce_constructor_arg_types(ta<Subject> subject,
                                            ta<TypesPool...> types_pool) {
  if constexpr (std::is_default_constructible_v<Subject>) {
    return empty_ta;
  } else {
    constexpr auto placeholders =
        detail::gen_constructor_arg_placeholders(subject, types_pool);
    if constexpr (placeholders == empty_ta) {
      static_assert(placeholders != empty_ta,
                    "Unable to detect any usable constructors. This may be "
                    "because there's no "
                    "accessible constructors at all, or constructor is declared "
                    "in some really unexpected way, or there is a constructor with more "
                    "than 32 arguments which is a little to much. Try to correct "
                    "these problems and compile again.");
      return error_ta;
    } else {
      return detail::gen_constructor_arg_types(subject, types_pool, empty_ta,
                                               decompose_tail(placeholders));
    }
  }
}

}  // namespace base::meta

#endif  // INJECXX_META_CONSTRUCTOR_HPP
