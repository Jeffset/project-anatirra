//
// Created by jeffset on 12/9/19.
//

#ifndef CURSEDUI_GFX_HPP
#define CURSEDUI_GFX_HPP

#include "base/macro.hpp"

#include <type_traits>

namespace cursedui::gfx {

using dim_t = int;

struct Point {
  dim_t x, y;
};

struct Size {
  dim_t width, height;
};

struct Rect {
  dim_t left, top, right, bottom;

  GETTER Point position() const;

  GETTER Size size() const;

  GETTER bool has_area() const;

  GETTER dim_t width() const;
  GETTER dim_t height() const;
};

Rect rect_from(const gfx::Point& position, const gfx::Size& size);

Size min(const Size& a, const Size& b);

Rect centered_rect(const Rect& base, const Size& size);

Rect grow(const Rect& base, dim_t d);
Rect shrink(const Rect& base, dim_t d);

}  // namespace cursedui::gfx

#endif  // CURSEDUI_GFX_HPP
