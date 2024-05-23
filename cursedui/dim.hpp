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

#include "base/enum_flags.hpp"
#include "base/macro.hpp"

#include "cursedui/config.hpp"

#include <cstdint>
#include <ostream>

namespace cursedui::gfx {

using dim_t = int;

enum class CURSEDUI_PUBLIC Gravity : uint8_t {
  // clang-format off
  LEFT =   0b0001,
  RIGHT =  0b0010,
  TOP =    0b0100,
  BOTTOM = 0b1000,

  CENTER = 0b1111,
  // clang-format on
};

struct CURSEDUI_PUBLIC Point {
  dim_t x, y;
};

struct CURSEDUI_PUBLIC Size {
  dim_t width, height;
};

struct CURSEDUI_PUBLIC Rect {
  dim_t left, top, right, bottom;

  GETTER Point position() const noexcept;

  GETTER Size size() const noexcept;

  GETTER bool has_area() const noexcept;

  GETTER dim_t width() const noexcept;
  GETTER dim_t height() const noexcept;

  bool contains(Point point) const noexcept;
  bool contains(const Rect& rect) const noexcept;

  bool intersects(const Rect& rhs) const noexcept;
};

// TODO: Convert the following functions into member ones for the ^^.

CURSEDUI_PUBLIC bool operator==(const Rect& lhs, const Rect& rhs) noexcept;
CURSEDUI_PUBLIC bool operator!=(const Rect& lhs, const Rect& rhs) noexcept;

CURSEDUI_PUBLIC bool operator==(const Size& lhs, const Size& rhs) noexcept;
CURSEDUI_PUBLIC bool operator!=(const Size& lhs, const Size& rhs) noexcept;

CURSEDUI_PUBLIC Rect rect_from(Point position, Size size) noexcept;

CURSEDUI_PUBLIC Size min(Size a, Size b) noexcept;

CURSEDUI_PUBLIC Size max(Size a, Size b) noexcept;

CURSEDUI_PUBLIC Rect centered_rect(const Rect& base, Size size) noexcept;

CURSEDUI_PUBLIC Rect grow(const Rect& base, dim_t d) noexcept;

CURSEDUI_PUBLIC Rect shrink(const Rect& base, dim_t d) noexcept;

CURSEDUI_PUBLIC Rect move(const Rect& base, const Size& offset) noexcept;

CURSEDUI_PUBLIC gfx::Rect gravitated_rect(const gfx::Rect& rect,
                                          gfx::Size size,
                                          base::EnumFlags<Gravity> gravity) noexcept;

CURSEDUI_PUBLIC std::ostream& operator<<(std::ostream& os, const Rect& rect);
CURSEDUI_PUBLIC std::ostream& operator<<(std::ostream& os, const Size& size);

}  // namespace cursedui::gfx
