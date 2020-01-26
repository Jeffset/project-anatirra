//
// Created by jeffset on 12/26/19.
//

#ifndef BASE_TYPE_ARRAY_HPP
#define BASE_TYPE_ARRAY_HPP

#include <type_traits>

namespace base::meta {

struct null_type {
  ~null_type() = delete;
};

struct error_type {
  ~error_type() = delete;
};

template <typename...>
class type_array {
 public:
  using single_type = null_type;
};

template <typename T>
class type_array<T> {
 public:
  using single_type = T;
};

template <typename... Ts>
using ta = type_array<Ts...>;

template <typename TA>
using ta_single_t = typename TA::single_type;

template <class T>
constexpr type_array<T> t{};

template <class... Ts>
constexpr type_array<Ts...> ts{};

constexpr ta<> empty_ta;

constexpr ta<null_type> null_ta;

constexpr ta<error_type> error_ta;

template <class... Ts>
constexpr int size(ta<Ts...>) {
  return sizeof...(Ts);
}

template <typename T, typename... Ts>
constexpr auto decompose_first(ta<T, Ts...>) {
  return t<T>;
}

template <typename T, typename... Ts>
constexpr auto decompose_tail(ta<T, Ts...>) {
  return ts<Ts...>;
}

template <typename... Ts1, typename... Ts2>
constexpr bool operator==(ta<Ts1...>, ta<Ts2...>) {
  if constexpr (sizeof...(Ts1) == sizeof...(Ts2))
    return (std::is_same_v<Ts1, Ts2> && ...);
  else
    return false;
}

template <typename... Ts1, typename... Ts2>
constexpr bool operator!=(ta<Ts1...>, ta<Ts2...>) {
  if constexpr (sizeof...(Ts1) == sizeof...(Ts2))
    return (!std::is_same_v<Ts1, Ts2> || ...);
  else
    return true;
}

template <typename... Ts1, typename... Ts2>
constexpr auto operator+(ta<Ts1...>, ta<Ts2...>) {
  return ts<Ts1..., Ts2...>;
}

template <typename... Ts>
constexpr auto reverse(ta<Ts...> array) {
  if constexpr (array == empty_ta) {
    return empty_ta;
  } else {
    return reverse(decompose_tail(array)) + decompose_first(array);
  }
}

template <class... Ts1, class... Ts2>
constexpr bool contains(ta<Ts1...>, ta<Ts2...>) {
  return ([]() {
    constexpr auto element = t<Ts1>;
    return ([element]() {
      if constexpr (t<Ts2> == element) {
        return true;
      } else {
        return false;
      }
    }() || ... ||
            false);
  }() || ... ||
          false);
}

template <class... Ts1, class... Ts2>
constexpr auto operator-(ta<Ts1...>, ta<Ts2...> what) {
  return ([what]() {
    if constexpr (contains(what, t<Ts1>)) {
      return empty_ta;
    } else {
      return t<Ts1>;
    }
  }() + ... +
          empty_ta);
}

template <typename Filter, typename... Ts>
constexpr auto filter(ta<Ts...>, Filter f) {
  return ([&]() {
    if constexpr (f(t<Ts>) == true) {
      return t<Ts>;
    } else {
      return empty_ta;
    }
  }() + ... +
          empty_ta);
}

template <typename Block, typename... Ts>
inline constexpr void for_each(ta<Ts...>, Block block) {
  (block(t<Ts>), ...);
}

template <typename Mapper, typename... Ts>
constexpr auto map(ta<Ts...>, Mapper mapper) {
  return (mapper(t<Ts>) + ... + empty_ta);
}

template <class... Ts>
constexpr auto wrap(ta<Ts...>) {
  return t<ta<Ts...>>;
}

template <class... Ts>
constexpr auto unwrap(ta<ta<Ts...>>) {
  return ts<Ts...>;
}

template <template <typename> typename P>
constexpr auto predicate() {
  return [](auto t) { return P<ta_single_t<decltype(t)>>::value; };
}

template <typename N>
constexpr auto not_type(ta<N>) {
  return [](auto t) { return !std::is_same_v<ta_single_t<decltype(t)>, N>; };
}

template <typename T>
constexpr auto is_type(ta<T>) {
  return [](auto t) { return std::is_same_v<ta_single_t<decltype(t)>, T>; };
}

template <typename T>
constexpr auto is_type() {
  return [](auto t) { return std::is_same_v<ta_single_t<decltype(t)>, T>; };
}

template <typename Base>
constexpr auto is_base_of(ta<Base>) {
  return [](auto t) { return std::is_base_of_v<Base, ta_single_t<decltype(t)>>; };
}

template <class... Ts>
constexpr auto filter_not_null(ta<Ts...> array) {
  return filter(array, not_type(null_ta));
}

}  // namespace base::meta

#endif  // BASE_TYPE_ARRAY_HPP
