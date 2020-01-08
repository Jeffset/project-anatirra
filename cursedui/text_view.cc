//
// Created by jeffset on 12/9/19.
//

#include "cursedui/text_view.hpp"

#include "base/util.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/rendering.hpp"

namespace cursedui::view {

namespace {

const gfx::dim_t MIN_HEIGHT = 1;

}  // namespace

void TextView::on_measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec) {
  gfx::Size measured_size = {
      std::visit(base::overloaded{
                     [this](MeasureUnlimited) { return width_(); },
                     [this](const MeasureAtMost& at_most) {
                       return std::min(at_most.dim, width_());
                     },
                     [](const MeasureExactly& exactly) { return exactly.dim; },
                 },
                 width_spec),
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
  text_pos_ = gfx::centered_rect(inner_bounds(), {width_(), 1}).position();
}

void TextView::on_colorize(render::ColorPalette& palette) {
  text_color_ = palette.obtain_color(render::RGB8Data{255, 255, 0});
}

render::BgColorState TextView::on_draw(render::Canvas& canvas) {
  auto bg = View::on_draw(canvas);
  // FIXME: width might be lesser than text size - handle that.
  canvas.set_foreground_color(text_color_.get());
  canvas << text_pos_ << text_.c_str();
  return bg;
}

void TextView::set_text(std::wstring_view str) {
  text_ = str;
}

int TextView::width_() const {
  return text_.size();
}

}  // namespace cursedui::view
