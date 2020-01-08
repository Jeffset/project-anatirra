//
// Created by jeffset on 12/15/19.
//

#ifndef BASE_UTIL_HPP
#define BASE_UTIL_HPP

#include <memory>
#include <optional>
#include <type_traits>

namespace base {

template <class T>
struct always_false : std::false_type {};

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

}  // namespace base

#endif  // BASE_UTIL_HPP
