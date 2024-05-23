/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

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

template <class E> requires std::is_enum_v<E>
constexpr EnumFlags<E> operator|(E lhs, E rhs) {
  return EnumFlags<E>(lhs) | EnumFlags<E>(rhs);
}

template <class E> requires std::is_enum_v<E>
constexpr EnumFlags<E> operator&(E lhs, E rhs) {
  return EnumFlags<E>(lhs) & EnumFlags<E>(rhs);
}

}  // namespace operators

}  // namespace base
