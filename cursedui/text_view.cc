//
// Created by jeffset on 12/9/19.
//

#include "cursedui/text_view.hpp"

#include "base/util.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/rendering.hpp"

#include <iostream>
#include <string_view>

namespace cursedui::view {

namespace {

const gfx::dim_t MIN_HEIGHT = 1;

}  // namespace

void TextView::on_measure(const MeasureSpec& text_len_spec,
                          const MeasureSpec& height_spec) {
  gfx::Size measured_size = {
      std::visit(base::overloaded{
                     [this](MeasureUnlimited) { return text_len_(); },
                     [this](const MeasureAtMost& at_most) {
                       return std::min(at_most.dim, text_len_());
                     },
                     [](const MeasureExactly& exactly) { return exactly.dim; },
                 },
                 text_len_spec),
      std::visit(base::overloaded{
                     [](MeasureUnlimited) { return MIN_HEIGHT; },
                     [](const MeasureAtMost& at_most) {
                       return std::min(MIN_HEIGHT, at_most.dim);
                     },
                     [](const MeasureExactly& exactly) { return exactly.dim; },
                 },
                 height_spec),
  };
  set_measured_size(measured_size);
}

void TextView::on_layout() {
  auto bounds = inner_bounds();
  ellipsize_ = text_len_() > bounds.width();
  std::wcerr << "JEFF = text: (" << text_ << ") " << text_len_()
             << " width: " << bounds.width() << '\n';
  text_to_render_ = text_;
  if (ellipsize_) {
    text_pos_ = gfx::centered_rect(bounds, {bounds.width(), 1}).position();
    auto shrink = text_len_() - bounds.width() + 1;
    text_to_render_.remove_suffix(shrink);
  } else {
    text_pos_ = gfx::centered_rect(bounds, {text_len_(), 1}).position();
  }
}

void TextView::on_colorize(render::ColorPalette& palette) {
  text_color_ = palette.obtain_color(render::RGB8Data{255, 255, 0});
}

render::BgColorState TextView::on_draw(render::Canvas& canvas) {
  auto bg = View::on_draw(canvas);
  // FIXME: width might be lesser than text size - handle that.
  canvas.set_foreground_color(text_color_.get());

  canvas << text_pos_ << text_to_render_;
  if (ellipsize_) {
    canvas << L'…';
  }
  return bg;
}

void TextView::set_text(std::wstring_view str) {
  text_ = str;
  text_to_render_ = str;
}

int TextView::text_len_() const {
  return text_.size();
}

}  // namespace cursedui::view
