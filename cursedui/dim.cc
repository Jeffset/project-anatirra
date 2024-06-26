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

#include "cursedui/dim.hpp"

#include "base/debug/debug.hpp"

#include <algorithm>

namespace cursedui::gfx {

Size min(Size a, Size b) noexcept {
  return {std::min(a.width, b.width), std::min(a.height, b.height)};
}

Size max(Size a, Size b) noexcept {
  return {std::max(a.width, b.width), std::max(a.height, b.height)};
}

Rect centered_rect(const Rect& base, Size size) noexcept {
  auto base_size = base.size();
  auto dx = base_size.width - size.width;
  dx = dx / 2 + dx % 2;
  auto dy = base_size.height - size.height;
  dy = dy / 2 + dy % 2;
  return Rect{
      base.left + dx,
      base.top + dy,
      base.right - dx,
      base.bottom - dy,
  };
}

Rect rect_from(Point position, Size size) noexcept {
  return Rect{
      position.x,
      position.y,
      position.x + size.width - 1,
      position.y + size.height - 1,
  };
}

Point Rect::position() const noexcept {
  return {left, top};
}

Size Rect::size() const noexcept {
  return {width(), height()};
}

Rect grow(const Rect& base, dim_t d) noexcept {
  return Rect{
      base.left - d,
      base.top - d,
      base.right + d,
      base.bottom + d,
  };
}

Rect shrink(const Rect& base, dim_t d) noexcept {
  return grow(base, -d);
}

Rect move(const Rect& base, const Size& offset) noexcept {
  return {base.left + offset.width, base.top + offset.height, base.right + offset.width,
          base.bottom + offset.height};
}

bool Rect::has_area() const noexcept {
  return right >= left && bottom >= top;
}

dim_t Rect::width() const noexcept {
  return right - left + 1;
}

dim_t Rect::height() const noexcept {
  return bottom - top + 1;
}

bool Rect::contains(Point point) const noexcept {
  auto [x, y] = point;
  return x >= left && x <= right && y >= top && y <= bottom;
}

bool Rect::contains(const Rect& r) const noexcept {
  return r.left >= left && r.top >= top && r.right <= right && r.bottom <= bottom;
}

bool Rect::intersects(const Rect& rhs) const noexcept {
  return right >= rhs.left && left <= rhs.right && top <= rhs.bottom && bottom >= rhs.top;
}

Rect gravitated_rect(const Rect& rect,
                     Size size,
                     base::EnumFlags<Gravity> gravity) noexcept {
  using namespace base::operators;

  dim_t left, right;
  const auto left_right = gravity & (Gravity::LEFT | Gravity::RIGHT);
  if (left_right == Gravity::LEFT) {
    left = rect.left;
    right = left + size.width - 1;
  } else if (left_right == Gravity::RIGHT) {
    right = rect.right;
    left = right - size.width + 1;
  } else {  // none or both => center
    auto dx = rect.width() - size.width;
    dx = dx / 2 + dx % 2;
    left = rect.left + dx;
    right = left + size.width - 1;  // rect.right - dx;
  }

  dim_t top, bottom;
  const auto top_bottom = gravity & (Gravity::TOP | Gravity::BOTTOM);
  if (top_bottom == Gravity::TOP) {
    top = rect.top;
    bottom = top + size.height - 1;
  } else if (top_bottom == Gravity::BOTTOM) {
    bottom = rect.bottom;
    top = bottom - size.height + 1;
  } else {  // none or both => center
    auto dy = rect.height() - size.height;
    dy = dy / 2 + dy % 2;
    top = rect.top + dy;
    bottom = top + size.height - 1;  // rect.bottom - dy;
  }

  auto r = Rect{left, top, right, bottom};
  ASSERT(r.width() == size.width);
  ASSERT(r.height() == size.height);
  return r;
}

bool operator==(const Rect& lhs, const Rect& rhs) noexcept {
  return std::tie(lhs.left, lhs.top, lhs.right, lhs.bottom) ==
         std::tie(rhs.left, rhs.top, rhs.right, rhs.bottom);
}

bool operator!=(const Rect& lhs, const Rect& rhs) noexcept {
  return std::tie(lhs.left, lhs.top, lhs.right, lhs.bottom) !=
         std::tie(rhs.left, rhs.top, rhs.right, rhs.bottom);
}

bool operator==(const Size& lhs, const Size& rhs) noexcept {
  return std::tie(lhs.width, lhs.height) == std::tie(rhs.width, rhs.height);
}

bool operator!=(const Size& lhs, const Size& rhs) noexcept {
  return std::tie(lhs.width, lhs.height) != std::tie(rhs.width, rhs.height);
}

std::ostream& operator<<(std::ostream& os, const Rect& rect) {
  return os << '[' << rect.left << ", " << rect.top << ", " << rect.right << ", "
            << rect.bottom << ']';
}

std::ostream& operator<<(std::ostream& os, const Size& size) {
  return os << '[' << size.width << "x" << size.height << ']';
}

}  // namespace cursedui::gfx
