//
// Created by jeffset on 12/15/19.
//

#ifndef CURSES_DEMO_UTIL_HPP
#define CURSES_DEMO_UTIL_HPP

#include <type_traits>


namespace cursedui::base {

template<class T>
struct always_false : std::false_type {};

template<class... Ts>
struct overloaded : Ts ... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

}

#endif //CURSES_DEMO_UTIL_HPP
