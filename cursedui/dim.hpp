//
// Created by jeffset on 12/9/19.
//

#ifndef CURSEDUI_GFX_HPP
#define CURSEDUI_GFX_HPP

#include "base/macro.hpp"

#include <cstdint>
#include <type_traits>

namespace cursedui::gfx {

using dim_t = int;

enum Gravity : uint8_t {
  // clang-format off
  GRAVITY_LEFT =   0b0001,
  GRAVITY_RIGHT =  0b0010,
  GRAVITY_TOP =    0b0100,
  GRAVITY_BOTTOM = 0b1000,

  GRAVITY_CENTER = 0b1111,
  // clang-format on
};

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

  bool contains(Point point) const;
};

Rect rect_from(Point position, Size size);

Size min(Size a, Size b);

Rect centered_rect(const Rect& base, Size size);

Rect grow(const Rect& base, dim_t d);

Rect shrink(const Rect& base, dim_t d);

gfx::Rect gravitated_rect(const gfx::Rect& rect, gfx::Size size, Gravity gravity);

}  // namespace cursedui::gfx

#endif  // CURSEDUI_GFX_HPP
