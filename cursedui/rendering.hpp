//
// Created by jeffset on 12/9/19.
//

#ifndef CURSES_DEMO_RENDERING_HPP
#define CURSES_DEMO_RENDERING_HPP

#include "gfx.hpp"

#include "macro.hpp"

#include <memory>


namespace cursedui {
class Context;
}

namespace cursedui::render {

struct BorderStyle;

struct BorderStyles {
  static const BorderStyle Single;
};

struct Box {
  gfx::Rect rect;
  BorderStyle const* style;
};

class Canvas {
 public:
  explicit Canvas(void*);
  ~Canvas();

  Canvas& operator<<(wchar_t ch);
  Canvas& operator<<(const wchar_t* str);
  Canvas& operator<<(const gfx::Point& pos);
  Canvas& operator<<(const Box& box);

  DISABLE_COPY_AND_ASSIGN(Canvas);

 private:
  struct CanvasImpl;
  std::unique_ptr<CanvasImpl> impl_;
};

void fill(Canvas& canvas, wchar_t ch, const gfx::Rect& area);

void border(Canvas& canvas, const gfx::Rect& rect, const BorderStyle& style);

}

#endif //CURSES_DEMO_RENDERING_HPP
