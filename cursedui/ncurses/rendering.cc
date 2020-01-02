//
// Created by jeffset on 12/9/19.
//

#include "cursedui/rendering.hpp"

#include <cassert>
#include <cursesw.h>

#ifdef border
#undef border
#endif

namespace cursedui::render {

struct BorderStyle {
  wchar_t top_left, top_right, bottom_right, bottom_left;
  wchar_t horizontal, vertical;
};

const BorderStyle BorderStyles::Single = {
    .horizontal = L'\u2500',
    .vertical = L'\u2502',

    .top_right = L'\u2510',
    .top_left = L'\u250C',
    .bottom_right = L'\u2518',
    .bottom_left = L'\u2514',
};

struct Canvas::CanvasImpl {};

Canvas::Canvas(void*) : impl_(new CanvasImpl()) {}

Canvas::~Canvas() = default;

Canvas& Canvas::operator<<(wchar_t ch) {
  ::waddnwstr(stdscr, &ch, 1);
  return *this;
}

Canvas& Canvas::operator<<(const wchar_t* str) {
  waddwstr(stdscr, str);
  return *this;
}

Canvas& Canvas::operator<<(const gfx::Point& pos) {
  auto r = ::wmove(stdscr, pos.y, pos.x);
  assert(r != ERR);
  return *this;
}

namespace {

void hor_line(gfx::dim_t length, wchar_t ch) {
  cchar_t cc[]{cchar_t{}, {}};
  wchar_t c[] = {ch, 0};
  ::setcchar(&cc[0], c, 0, 0, nullptr);

  ::whline_set(stdscr, cc, length);
}

void ver_line(gfx::dim_t length, wchar_t ch) {
  cchar_t cc[]{cchar_t{}, {}};
  wchar_t c[] = {ch, 0};
  ::setcchar(&cc[0], c, 0, 0, nullptr);

  ::wvline_set(stdscr, cc, length);
}

}  // namespace

void fill(Canvas& canvas, wchar_t ch, const gfx::Rect& area) {
  auto y = area.top;
  for (; y <= area.bottom; ++y) {
    auto x = area.left;
    canvas << gfx::Point{x, y};
    for (; x <= area.right; ++x) {
      canvas << ch;
    }
  }
}

void border(Canvas& canvas, const gfx::Rect& rect, const BorderStyle& style) {
  canvas << gfx::Point{rect.left, rect.top} << style.top_left;
  canvas << gfx::Point{rect.left, rect.bottom} << style.bottom_left;
  canvas << gfx::Point{rect.right, rect.top} << style.top_right;
  canvas << gfx::Point{rect.right, rect.bottom} << style.bottom_right;

  auto size = rect.size();
  if (size.width > 2) {
    canvas << gfx::Point{rect.left + 1, rect.top};
    hor_line(size.width - 2, style.horizontal);
    canvas << gfx::Point{rect.left + 1, rect.bottom};
    hor_line(size.width - 2, style.horizontal);
  }
  if (size.height > 2) {
    canvas << gfx::Point{rect.left, rect.top + 1};
    ver_line(size.height - 2, style.vertical);
    canvas << gfx::Point{rect.right, rect.top + 1};
    ver_line(size.height - 2, style.vertical);
  }
}

}  // namespace cursedui::render
