// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_META_STRING
#define ANATIRRA_BASE_META_STRING

#include "base/type_array.hpp"

namespace base {

template <char... Chars>
struct meta_str {
  static constexpr const char value[sizeof...(Chars) + 1] = {Chars..., '\0'};
  static constexpr int size = sizeof...(Chars);
};

template <char... Chars1, char... Chars2>
constexpr auto operator+(meta_str<Chars1...>, meta_str<Chars2...>) {
  return meta_str<Chars1..., Chars2...>{};
}

template <char... Chars1, char... Chars2>
constexpr auto operator==(meta_str<Chars1...>, meta_str<Chars2...>) {
  auto cmp = [](char c1, char c2) { return c1 == c2; };
  return (cmp(Chars1, Chars2) && ... && true);
}

template <char... Chars1, char... Chars2>
constexpr auto operator!=(meta_str<Chars1...>, meta_str<Chars2...>) {
  auto cmp = [](char c1, char c2) { return c1 != c2; };
  return (cmp(Chars1, Chars2) || ... || false);
}

template <char... Chars, class Block>
constexpr void for_each_char(meta_str<Chars...>, Block block) {
  (block(meta_str<Chars>{}), ...);
}

template <char... Chars, class Block>
constexpr auto map(meta_str<Chars...>, Block transformer) {
  return (transformer(meta_str<Chars>{}) + ... + meta_str<>{});
}

template <char C, char... Rest>
constexpr auto first(meta_str<C, Rest...>) {
  return meta_str<C>{};
}

template <char C, char... Rest>
constexpr auto tail(meta_str<C, Rest...>) {
  return meta_str<Rest...>{};
}

namespace lang {

enum class Token {
  NUMBER,
  OP_PLUS,
  OP_MINUS,
};

template <char... Chars>
constexpr auto lex(meta_str<Chars...> source) {}

}  // namespace lang

namespace meta_literals {

template <typename CharT, CharT... String>
constexpr meta_str<String...> operator"" _meta() {
  return {};
}

}  // namespace meta_literals

}  // namespace base

#endif  // ANATIRRA_BASE_META_STRING
