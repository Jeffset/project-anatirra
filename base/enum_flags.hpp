// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_ENUM_FLAGS
#define ANATIRRA_BASE_ENUM_FLAGS

#include "base/macro.hpp"

#include <initializer_list>
#include <type_traits>

namespace base {

template <class E>
class EnumFlags {
  static_assert(std::is_enum_v<E>);
  using flags_t = std::underlying_type_t<E>;

 private:
  constexpr explicit EnumFlags(flags_t flags) noexcept : flags_(flags) {}

 public:
  constexpr EnumFlags() noexcept = default;

  constexpr EnumFlags(E single) noexcept : flags_(static_cast<flags_t>(single)) {}

  constexpr EnumFlags(std::initializer_list<E> flags) noexcept : flags_(0) {
    for (auto flag : flags)
      flags_ |= static_cast<flags_t>(flag);
  }

  constexpr EnumFlags<E> has(E flag) const noexcept {
    return EnumFlags(flags_ & static_cast<flags_t>(flag));
  }
  constexpr EnumFlags<E> has(EnumFlags<E> flags) const noexcept {
    return EnumFlags(flags_ & flags.flags_);
  }

  constexpr EnumFlags& add(E flag) noexcept {
    flags_ |= static_cast<flags_t>(flag);
    return *this;
  }
  constexpr EnumFlags& add(EnumFlags flags) noexcept {
    flags_ |= flags.flags_;
    return *this;
  }

  constexpr EnumFlags& remove(E flag) noexcept {
    flags_ &= ~static_cast<flags_t>(flag);
    return *this;
  }
  constexpr EnumFlags& remove(EnumFlags<E> flags) noexcept {
    flags_ &= ~flags.flags_;
    return *this;
  }

  friend constexpr EnumFlags operator^(EnumFlags lhs, EnumFlags rhs) noexcept {
    return EnumFlags(lhs.flags_ ^ rhs.flags_);
  }

  friend constexpr EnumFlags operator|(EnumFlags lhs, EnumFlags rhs) noexcept {
    return EnumFlags(lhs.flags_ | rhs.flags_);
  }

  friend constexpr EnumFlags operator&(EnumFlags lhs, EnumFlags rhs) noexcept {
    return EnumFlags(lhs.flags_ & rhs.flags_);
  }

  friend constexpr bool operator==(EnumFlags lhs, EnumFlags rhs) noexcept {
    return lhs.flags_ == rhs.flags_;
  }

  friend constexpr bool operator!=(EnumFlags lhs, EnumFlags rhs) noexcept {
    return lhs.flags_ != rhs.flags_;
  }

  constexpr operator bool() const noexcept { return flags_; }

 private:
  flags_t flags_;
};

namespace operators {

template <class E, REQUIRES(std::is_enum_v<E>)>
constexpr EnumFlags<E> operator|(E lhs, E rhs) {
  return EnumFlags<E>(lhs) | EnumFlags<E>(rhs);
}

template <class E, REQUIRES(std::is_enum_v<E>)>
constexpr EnumFlags<E> operator&(E lhs, E rhs) {
  return EnumFlags<E>(lhs) & EnumFlags<E>(rhs);
}

}  // namespace operators

}  // namespace base

#endif  // ANATIRRA_BASE_ENUM_FLAGS
