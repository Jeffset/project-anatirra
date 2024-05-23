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

#include "avada/config.hpp"
#include "base/macro.hpp"

#include <cstdint>
#include <bit>
#include <variant>

namespace avada::render {

class AVADA_PUBLIC ColorRGB {
public:
  using channel_t = uint8_t;

  constexpr ColorRGB(channel_t red, channel_t green, channel_t blue, channel_t alpha = 255) noexcept
    : rgba_(std::bit_cast<uint32_t, uint8_t[4]>({red, green, blue, alpha})) {
  }

  constexpr explicit ColorRGB(uint32_t rgb) noexcept : rgba_{rgb} {}
  constexpr ColorRGB() : rgba_{} {}

  GETTER constexpr channel_t red() const noexcept { return as_array()[0]; }
  GETTER constexpr channel_t green() const noexcept { return as_array()[1]; }
  GETTER constexpr channel_t blue() const noexcept { return as_array()[2]; }
  GETTER constexpr channel_t alpha() const noexcept { return as_array()[3]; }

  GETTER constexpr channel_t& red() noexcept { return as_array()[0]; }
  GETTER constexpr channel_t& green() noexcept { return as_array()[1]; }
  GETTER constexpr channel_t& blue() noexcept { return as_array()[2]; }
  GETTER constexpr channel_t& alpha() noexcept { return as_array()[3]; }

  GETTER constexpr uint32_t& color_int() noexcept { return rgba_; }
  GETTER constexpr const uint32_t& color_int() const noexcept { return rgba_; }

  constexpr auto operator<=>(const ColorRGB& rhs) const noexcept = default;
private:
  const channel_t* as_array() const noexcept {
    return reinterpret_cast<const channel_t*>(&rgba_);
  }

  channel_t* as_array() noexcept {
    return reinterpret_cast<channel_t*>(&rgba_);
  }

  uint32_t rgba_;
};

struct AVADA_PUBLIC Colors {
  Colors() = delete;

  static const ColorRGB TRANSPARENT;
  static const ColorRGB BLACK;
  static const ColorRGB WHITE;
};

struct AVADA_PUBLIC RenderAttributes {
  RenderAttributes() = delete;

  static constexpr uint8_t BOLD = 1 << 0;
  static constexpr uint8_t ITALIC = 1 << 1;
  static constexpr uint8_t UNDERLINE = 1 << 2;
};

enum class AVADA_PUBLIC SystemColor : uint8_t {
  BLACK = 0,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  DEFAULT,
};

using Color = std::variant<ColorRGB, SystemColor>;

AVADA_PUBLIC
Color alpha_blend(const Color& source, const Color& destination) noexcept;

}  // namespace avada::render
