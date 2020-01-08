#include "cursedui/drawable.hpp"

#include "cursedui/rendering.hpp"

#include <cassert>

namespace cursedui {

Drawable::Drawable() noexcept = default;

Drawable::~Drawable() noexcept = default;

void Drawable::set_bounds(const gfx::Rect& bounds) {
  bounds_ = bounds;
}

void Drawable::colorize(render::ColorPalette& palette) {
  background_ = palette.obtain_color(render::Color::BLACK);
}

SolidColorDrawable::SolidColorDrawable() noexcept : color_descr_(render::Color::BLACK) {}

SolidColorDrawable::SolidColorDrawable(render::ColorDescr color_descr) noexcept
    : color_descr_(color_descr) {}

void SolidColorDrawable::colorize(render::ColorPalette& palette) {
  background_ = palette.obtain_color(color_descr_);
}

render::BgColorState SolidColorDrawable::draw(render::Canvas& canvas) {
  if (!bounds().has_area())
    return render::BgColorState{};
  auto _bg = canvas.set_background_color(background_.get());
  render::fill(canvas, ' ', bounds());
  return _bg;
}

BorderDrawable::BorderDrawable() noexcept
    : color_descr_(render::Color::WHITE), background_descr_(), style_(SINGLE) {}

void BorderDrawable::set_style(BorderDrawable::Style style) {
  if (style_ == style)
    return;
  style_ = style;
  if (style_ == NO_BORDER) {
    background_ = nullptr;
    foreground_ = nullptr;
  }
}

void BorderDrawable::set_background_color(std::optional<render::ColorDescr> color_descr) {
  background_descr_ = color_descr;
  if (!background_descr_.has_value()) {
    background_ = nullptr;
  }
}

gfx::dim_t BorderDrawable::border_width() const {
  switch (style_) {
    case SINGLE:
      return 1;
    case DOUBLE:
      return 2;
    case NO_BORDER:
      return 0;
    default:
      assert(false);
  }
}

void BorderDrawable::colorize(render::ColorPalette& palette) {
  if (style_ != NO_BORDER) {
    if (background_descr_.has_value())
      background_ = palette.obtain_color(background_descr_.value());
    foreground_ = palette.obtain_color(color_descr_);
  }
}

render::BgColorState BorderDrawable::draw(render::Canvas& canvas) {
  if (style_ == NO_BORDER || !bounds().has_area())
    return render::BgColorState{};
  // FIXME: use border style
  render::BgColorState bg;
  if (background_) {
    bg = canvas.set_background_color(background_.get());
  }
  canvas.set_foreground_color(foreground_.get());
  canvas << render::Box{bounds(), &render::BorderStyles::Single};
  return bg;
}

}  // namespace cursedui
