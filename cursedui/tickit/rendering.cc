//
// Created by jeffset on 12/9/19.
//

#include "cursedui/rendering.hpp"

#include "tickit.h"

#include <cassert>

#ifdef border
#undef border
#endif

namespace cursedui::render {

struct BorderStyle {
  TickitLineStyle style_;
};

const BorderStyle BorderStyles::Single = {TICKIT_LINE_SINGLE};

struct Canvas::CanvasImpl {
  TickitRenderBuffer* render_buffer_;
};

Canvas::Canvas(void* rb) : impl_(new CanvasImpl()) {
  impl_->render_buffer_ = static_cast<TickitRenderBuffer*>(rb);
}

Canvas::~Canvas() = default;

Canvas& Canvas::operator<<(wchar_t ch) {
  tickit_renderbuffer_char(impl_->render_buffer_, ch);
  return *this;
}

Canvas& Canvas::operator<<(const wchar_t* str) {
  auto* ch = str;
  while (*ch) {
    tickit_renderbuffer_char(impl_->render_buffer_, *(ch++));
  }
  return *this;
}

Canvas& Canvas::operator<<(const gfx::Point& pos) {
  tickit_renderbuffer_goto(impl_->render_buffer_, pos.y, pos.x);
  return *this;
}

Canvas& Canvas::operator<<(const Box& box) {
  auto* rb = impl_->render_buffer_;
  tickit_renderbuffer_hline_at(rb, box.rect.top, box.rect.left, box.rect.right,
                               box.style->style_, (TickitLineCaps)0);
  tickit_renderbuffer_hline_at(rb, box.rect.bottom, box.rect.left, box.rect.right,
                               box.style->style_, (TickitLineCaps)0);
  tickit_renderbuffer_vline_at(rb, box.rect.top, box.rect.bottom, box.rect.left,
                               box.style->style_, (TickitLineCaps)0);
  tickit_renderbuffer_vline_at(rb, box.rect.top, box.rect.bottom, box.rect.right,
                               box.style->style_, (TickitLineCaps)0);
  return *this;
}

void fill(Canvas& canvas, wchar_t ch, const gfx::Rect& area) {
  // TODO: use erase if ch is whitespace.
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
  canvas << Box{rect, &style};
}

}  // namespace cursedui::render
