// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/dim.hpp"

#include <algorithm>

namespace cursedui::gfx {

Size min(Size a, Size b) {
  return {std::min(a.width, b.width), std::min(a.height, a.height)};
}

Size max(Size a, Size b) {
  return {std::max(a.width, b.width), std::max(a.height, a.height)};
}

Rect centered_rect(const Rect& base, Size size) {
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

Rect rect_from(Point position, Size size) {
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

bool Rect::contains(Point point) const {
  auto [x, y] = point;
  return x >= left && x <= right && y >= top && y <= bottom;
}

Rect gravitated_rect(const Rect& rect, Size size, Gravity gravity) {
  dim_t left, right;
  switch (gravity & (GRAVITY_LEFT | GRAVITY_RIGHT)) {
    case GRAVITY_LEFT:
      left = rect.left;
      right = left + size.width - 1;
      break;
    case GRAVITY_RIGHT:
      right = rect.right;
      left = right - size.width + 1;
      break;
    case 0:
    case GRAVITY_LEFT | GRAVITY_RIGHT: {
      auto dx = rect.width() - size.width;
      dx = dx / 2 + dx % 2;
      left = rect.left + dx;
      right = rect.right - dx;
    } break;
    default:
      // TODO: change to proper exception type
      throw std::exception();
  }

  dim_t top, bottom;
  switch (gravity & (GRAVITY_TOP | GRAVITY_BOTTOM)) {
    case GRAVITY_TOP:
      top = rect.top;
      bottom = top + size.height - 1;
      break;
    case GRAVITY_BOTTOM:
      bottom = rect.bottom;
      top = bottom - size.height + 1;
      break;
    case 0:
    case GRAVITY_TOP | GRAVITY_BOTTOM: {
      auto dy = rect.height() - size.height;
      dy = dy / 2 + dy % 2;
      top = rect.top + dy;
      bottom = rect.bottom - dy;
    } break;
    default:
      // TODO: change to proper exception type
      throw std::exception();
  }

  return Rect{left, top, right, bottom};
}

}  // namespace cursedui::gfx
