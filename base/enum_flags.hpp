// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_ENUM_FLAGS
#define ANATIRRA_BASE_ENUM_FLAGS

#include <initializer_list>
#include <type_traits>

namespace base {

template <class E>
class EnumFlags {
  static_assert(std::is_enum_v<E>);
  using flags_t = std::underlying_type_t<E>;

 private:
  constexpr EnumFlags(flags_t flags) noexcept : flags_(flags) {}

 public:
  constexpr EnumFlags() noexcept = default;

  constexpr explicit EnumFlags(E single) noexcept
      : flags_(static_cast<flags_t>(single)) {}

  constexpr EnumFlags(std::initializer_list<E> flags) noexcept : flags_(0) {
    for (auto flag : flags)
      flags_ |= static_cast<flags_t>(flag);
  }

  constexpr bool has(E flag) const noexcept {
    return flags_ & static_cast<flags_t>(flag);
  }
  constexpr bool has(EnumFlags<E> flags) const noexcept {
    return flags_ & static_cast<flags_t>(flags.flags_);
  }

  constexpr void set(E flag) noexcept { flags_ |= static_cast<flags_t>(flag); }
  void set(EnumFlags<E> flags) noexcept { flags_ |= flags.flags_; }

  constexpr void clear(E flag) noexcept { flags_ &= ~static_cast<flags_t>(flag); }
  constexpr void clear(EnumFlags<E> flags) noexcept {
    flags_ &= ~static_cast<flags_t>(flags.flags_);
  }

  constexpr EnumFlags& operator<<(E flag) noexcept {
    set(flag);
    return *this;
  }

  constexpr EnumFlags& operator+=(E flag) noexcept {
    set(flag);
    return *this;
  }

  constexpr EnumFlags& operator-=(E flag) noexcept {
    clear(flag);
    return *this;
  }

  constexpr EnumFlags operator^(EnumFlags rhs) const noexcept {
    return EnumFlags(flags_ ^ rhs.flags_);
  }

 private:
  flags_t flags_;
};

}  // namespace base

#endif  // ANATIRRA_BASE_ENUM_FLAGS
