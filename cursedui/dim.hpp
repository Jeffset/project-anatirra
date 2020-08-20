// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_DIM
#define ANATIRRA_CURSEDUI_DIM

#include "base/enum_flags.hpp"
#include "base/macro.hpp"

#include "cursedui_config.hpp"

#include <cstdint>
#include <ostream>
#include <tuple>
#include <type_traits>

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

#endif  // ANATIRRA_CURSEDUI_DIM
