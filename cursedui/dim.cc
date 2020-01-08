//
// Created by jeffset on 12/9/19.
//

#include "cursedui/dim.hpp"

#include <algorithm>

namespace cursedui::gfx {

Size min(const Size& a, const Size& b) {
  return {std::min(a.width, b.width), std::min(a.height, a.height)};
}

Rect centered_rect(const Rect& base, const Size& size) {
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

Rect rect_from(const gfx::Point& position, const gfx::Size& size) {
  return Rect{
      position.x,
      position.y,
      position.x + size.width - 1,
      position.y + size.height - 1,
  };
}

Point Rect::position() const {
  return {left, top};
}

Size Rect::size() const {
  return {width(), height()};
}

Rect grow(const Rect& base, dim_t d) {
  return Rect{
      base.left - d,
      base.top - d,
      base.right + d,
      base.bottom + d,
  };
}

Rect shrink(const Rect& base, dim_t d) {
  return grow(base, -d);
}

bool Rect::has_area() const {
  return right >= left && bottom >= top;
}

dim_t Rect::width() const {
  return right - left + 1;
}

dim_t Rect::height() const {
  return bottom - top + 1;
}

}  // namespace cursedui::gfx
