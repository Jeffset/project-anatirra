// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_DIM
#define ANATIRRA_CURSEDUI_DIM

#include "base/enum_flags.hpp"
#include "base/macro.hpp"

#include <cstdint>
#include <type_traits>

namespace cursedui::gfx {

using dim_t = int;

enum class Gravity : uint8_t {
  // clang-format off
  LEFT =   0b0001,
  RIGHT =  0b0010,
  TOP =    0b0100,
  BOTTOM = 0b1000,

  CENTER = 0b1111,
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

  GETTER Point position() const noexcept;

  GETTER Size size() const noexcept;

  GETTER bool has_area() const noexcept;

  GETTER dim_t width() const noexcept;
  GETTER dim_t height() const noexcept;

  bool contains(Point point) const;
};

Rect rect_from(Point position, Size size) noexcept;

Size min(Size a, Size b) noexcept;

Size max(Size a, Size b) noexcept;

Rect centered_rect(const Rect& base, Size size) noexcept;

Rect grow(const Rect& base, dim_t d) noexcept;

Rect shrink(const Rect& base, dim_t d) noexcept;

gfx::Rect gravitated_rect(const gfx::Rect& rect,
                          gfx::Size size,
                          base::EnumFlags<Gravity> gravity) noexcept;

}  // namespace cursedui::gfx

#endif  // ANATIRRA_CURSEDUI_DIM
