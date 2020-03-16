// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/rendering.hpp"

#include "tickit.h"

#include <cassert>
#include <cmath>

namespace cursedui::render {

struct Color::ColorImpl {
  TickitPenRGB8 color;
};

Color::Color(float r, float g, float b)
    : impl_(new ColorImpl({(uint8_t)std::round(255 * r), (uint8_t)std::round(255 * g),
                           (uint8_t)std::round(255 * b)})) {}

Color::Color(float gray) : Color(gray, gray, gray) {}

Color::Color(Color&& color) noexcept = default;

Color::Color(const Color& color) noexcept : impl_(new ColorImpl({color.impl_->color})) {}

Color& Color::operator=(const Color& color) noexcept {
  impl_->color = color.impl_->color;
  return *this;
}

Color& Color::operator=(Color&& color) noexcept = default;

Color::~Color() = default;

struct BorderStyle {
  TickitLineStyle style_;
};

const BorderStyle BorderStyles::Single = {TICKIT_LINE_SINGLE};

struct Canvas::CanvasImpl {
  TickitRenderBuffer* render_buffer_;
  TickitPen* pen_;
};

Canvas::Canvas(void* rb) : impl_(new CanvasImpl()) {
  impl_->render_buffer_ = static_cast<TickitRenderBuffer*>(rb);
  impl_->pen_ = tickit_pen_new();
  tickit_pen_set_colour_attr_desc(impl_->pen_, TICKIT_PEN_FG, "red");
  tickit_pen_set_colour_attr_rgb8(impl_->pen_, TICKIT_PEN_FG, TickitPenRGB8{128, 128, 0});
  //                                  Color(1.f, 0.f, 1.f).impl_->color);
  tickit_renderbuffer_setpen(impl_->render_buffer_, impl_->pen_);
  assert(tickit_pen_has_colour_attr_rgb8(impl_->pen_, TICKIT_PEN_FG));
}

Canvas::~Canvas() = default;

// static
void Canvas::init_rendering() {
  // no need for static initialization of tickit.
}

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

Canvas& Canvas::operator<<(const Color& color) {
  //  tickit_pen_set_colour_attr_desc(impl_->pen_, TICKIT_PEN_FG, "cyan");
  //  tickit_pen_set_colour_attr_rgb8(impl_->pen_, TICKIT_PEN_FG, color.impl_->color);
  //  tickit_renderbuffer_setpen(impl_->render_buffer_, impl_->pen_);
  //  assert(tickit_pen_has_colour_attr_rgb8(impl_->pen_, TICKIT_PEN_FG));
  return *this;
}

Canvas& Canvas::operator<<(Color&& color) {
  //  tickit_pen_set_colour_attr_desc(impl_->pen_, TICKIT_PEN_FG, "cyan");
  //  tickit_pen_set_colour_attr_rgb8(impl_->pen_, TICKIT_PEN_FG, color.impl_->color);
  //  tickit_renderbuffer_setpen(impl_->render_buffer_, impl_->pen_);
  //  assert(tickit_pen_has_colour_attr_rgb8(impl_->pen_, TICKIT_PEN_FG));
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
