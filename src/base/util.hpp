//
// Created by jeffset on 12/15/19.
//

#ifndef BASE_UTIL_HPP
#define BASE_UTIL_HPP

#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

namespace base {

template <class T>
struct always_false : std::false_type {};

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

template <class TargetVariant, class... SourceVariants>
TargetVariant subvariant(const std::variant<SourceVariants...>& source) {
  return std::visit([](const auto& s) -> TargetVariant { return s; }, source);
}

template <class Type, class... VariantTypes>
bool holds_alternative(
    const std::optional<std::variant<VariantTypes...>>& variant) noexcept {
  return variant.has_value() && std::holds_alternative<Type>(variant.value());
}

template <class Type, class... VariantTypes>
inline bool holds_alternative(const std::variant<VariantTypes...>& variant) noexcept {
  return std::holds_alternative<Type>(variant);
}

template <class T>
constexpr auto identity_map = [](T v) { return v; };

}  // namespace base

#endif  // BASE_UTIL_HPP
