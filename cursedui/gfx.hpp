//
// Created by jeffset on 12/9/19.
//

#ifndef CURSES_DEMO_GFX_HPP
#define CURSES_DEMO_GFX_HPP

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

  [[nodiscard]] Point position() const;

  [[nodiscard]] Size size() const;

  [[nodiscard]] bool has_area() const;
};

Rect rect_from(const gfx::Point& position, const gfx::Size& size);

Size min(const Size& a, const Size& b);

Rect centered_rect(const Rect& base, const Size& size);

Rect grow(const Rect& base, dim_t d);
Rect shrink(const Rect& base, dim_t d);

}

#endif //CURSES_DEMO_GFX_HPP
