//
// Created by jeffset on 12/9/19.
//

#include "cursedui/gfx.hpp"

#include <algorithm>

namespace cursedui::gfx {

Size min(const Size& a, const Size& b) {
  return {.width = std::min(a.width, b.width),
          .height = std::min(a.height, a.height)};
}

Rect centered_rect(const Rect& base, const Size& size) {
  auto base_size = base.size();
  auto dx = base_size.width - size.width;
  dx = dx / 2 + dx % 2;
  auto dy = base_size.height - size.height;
  dy = dy / 2 + dy % 2;
  return {
      .left = base.left + dx,
      .top = base.top + dy,
      .right = base.right - dx,
      .bottom = base.bottom - dy,
  };
}

Rect rect_from(const gfx::Point& position, const gfx::Size& size) {
  return {
      .left = position.x,
      .top = position.y,
      .right = position.x + size.width - 1,
      .bottom = position.y + size.height - 1,
  };
}

Point Rect::position() const {
  return {left, top};
}

Size Rect::size() const {
  return {right - left + 1, bottom - top + 1};
}

Rect grow(const Rect& base, dim_t d) {
  return {
      .left = base.left - d,
      .right = base.right + d,
      .bottom = base.bottom + d,
      .top = base.top - d,
  };
}

Rect shrink(const Rect& base, dim_t d) {
  return grow(base, -d);
}

bool Rect::has_area() const {
  return right >= left && bottom >= top;
}

}  // namespace cursedui::gfx
