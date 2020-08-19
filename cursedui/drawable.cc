// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/drawable.hpp"

#include "avada/color.hpp"
#include "base/debug/debug.hpp"

#include <array>

namespace cursedui {

namespace {

struct CURSEDUI_PRIVATE BorderSet {
  wchar_t top_left;
  wchar_t top_right;
  wchar_t bottom_left;
  wchar_t bottom_right;

  wchar_t horizontal;
  wchar_t vertical;

  void draw(paint::Canvas& canvas,
            const paint::Pen& pen,
            const gfx::Rect& bounds) const noexcept {
    canvas.draw(top_left, {bounds.left, bounds.top}, pen);
    canvas.draw(top_right, {bounds.right, bounds.top}, pen);
    canvas.draw(bottom_left, {bounds.left, bounds.bottom}, pen);
    canvas.draw(bottom_right, {bounds.right, bounds.bottom}, pen);
    const auto size = bounds.size();

    canvas.draw_line(horizontal, {bounds.left + 1, bounds.top},
                     paint::Direction::HORIZONTAL, size.width - 2, pen);
    canvas.draw_line(horizontal, {bounds.left + 1, bounds.bottom},
                     paint::Direction::HORIZONTAL, size.width - 2, pen);

    canvas.draw_line(vertical, {bounds.left, bounds.top + 1}, paint::Direction::VERTICAL,
                     size.height - 2, pen);
    canvas.draw_line(vertical, {bounds.right, bounds.top + 1}, paint::Direction::VERTICAL,
                     size.height - 2, pen);
  }
};

const BorderSet SINGLE_BORDER{L'┌', L'┐', L'└', L'┘', L'─', L'│'};
const BorderSet DOUBLE_BORDER{L'╔', L'╗', L'╚', L'╝', L'═', L'║'};

}  // namespace

Drawable::Drawable() noexcept = default;
Drawable::~Drawable() noexcept = default;

SolidColorDrawable::SolidColorDrawable() noexcept
    : SolidColorDrawable(avada::render::SystemColor::DEFAULT) {}

SolidColorDrawable::SolidColorDrawable(avada::render::Color color) noexcept
    : pen_(avada::render::Colors::TRANSPARENT, color) {}

void SolidColorDrawable::set_color(avada::render::Color color) noexcept {
  if (pen_.bg_color == color) {
    return;
  }
  pen_.bg_color = color;
  mark_needs_repaint();
}

void SolidColorDrawable::draw(paint::Canvas& canvas, const gfx::Rect& bounds) noexcept {
  if (!bounds.has_area())
    return;
  canvas.fill(' ', bounds, pen_);
}

BorderDrawable::BorderDrawable() noexcept
    : pen_(avada::render::SystemColor::DEFAULT, avada::render::Colors::TRANSPARENT),
      style_(Style::SINGLE) {}

void BorderDrawable::set_style(BorderDrawable::Style style) noexcept {
  if (style_ == style)
    return;
  const auto old_width = border_width();
  style_ = style;
  mark_needs_repaint();
  if (old_width != border_width()) {
    mark_needs_layout(view::NeedsLayout::SIZE);
  }
}

void BorderDrawable::set_color(avada::render::Color color) noexcept {
  if (pen_.fg_color == color) {
    return;
  }
  pen_.fg_color = color;
  mark_needs_repaint();
}

void BorderDrawable::set_background_color(avada::render::Color color) noexcept {
  if (pen_.bg_color == color) {
    return;
  }
  pen_.bg_color = color;
  mark_needs_repaint();
}

gfx::dim_t BorderDrawable::border_width() const noexcept {
  switch (style_) {
    case Style::SINGLE:
    case Style::DOUBLE:
      return 1;
    case Style::NO_BORDER:
      return 0;
    default:
      ASSERT(false) << "Not reached";
  }
}

void BorderDrawable::draw(paint::Canvas& canvas, const gfx::Rect& bounds) noexcept {
  if (style_ == Style::NO_BORDER || !bounds.has_area())
    return;

  switch (style_) {
    case Style::SINGLE:
      SINGLE_BORDER.draw(canvas, pen_, bounds);
      break;
    case Style::DOUBLE:
      DOUBLE_BORDER.draw(canvas, pen_, bounds);
      break;
    default:
      ASSERT(false) << "Not reached";
  }
}

}  // namespace cursedui
