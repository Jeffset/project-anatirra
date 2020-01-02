//
// Created by jeffset on 12/7/19.
//

#ifndef CURSES_DEMO_MACRO_HPP
#define CURSES_DEMO_MACRO_HPP

#define DISABLE_COPY_AND_ASSIGN(Class) \
Class(const Class&) = delete; \
Class& operator=(const Class&) = delete

#define GETTER [[nodiscard]]

#endif //CURSES_DEMO_MACRO_HPP
