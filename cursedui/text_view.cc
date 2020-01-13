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
  // TODO: Handle '\n' in text properly.
  auto bounds = inner_bounds();
  auto [width, height] = bounds.size();
  auto text_len = text_len_();
  if (multiline_) {
    auto lines_count = text_len / width;
    if (text_len % width) {
      ++lines_count;
    }
    ellipsize_ = lines_count > height;
    lines_count = std::min(height, lines_count);
    auto text_size = gfx::Size{std::min(text_len, width), lines_count};
    lines_to_render_.clear();
    for (int i = 0; i < lines_count; ++i) {
      std::wstring_view line = text_;
      line.remove_prefix(i * width);
      if (ellipsize_ || i != (lines_count - 1)) {
        line.remove_suffix(text_len - (i + 1) * width);
      }
      lines_to_render_.push_back(line);
    }
    if (ellipsize_) {
      // this is for '...' symbol.
      lines_to_render_.back().remove_suffix(1);
    }
    text_pos_ = gfx::gravitated_rect(bounds, text_size, gravity_).position();
  } else {
    ellipsize_ = text_len > width;
    std::wstring_view text_to_render = text_;
    if (ellipsize_) {
      text_pos_ = gfx::gravitated_rect(bounds, {width, 1}, gravity_).position();
      auto shrink = text_len - width + 1;
      text_to_render.remove_suffix(shrink);
    } else {
      text_pos_ = gfx::gravitated_rect(bounds, {text_len, 1}, gravity_).position();
    }
    lines_to_render_ = {text_to_render};
  }
}

void TextView::on_colorize(render::ColorPalette& palette) {
  text_color_ = palette.obtain_color(render::RGB8Data{255, 255, 0});
}

render::BgColorState TextView::on_draw(render::Canvas& canvas) {
  auto bg = View::on_draw(canvas);
  canvas.set_foreground_color(text_color_.get());

  auto pos = text_pos_;
  for (auto& line : lines_to_render_) {
    canvas << pos << line;
    ++pos.y;
  }
  if (ellipsize_) {
    canvas << L'…';
  }
  return bg;
}

TextView::TextView()
    : gravity_(static_cast<gfx::Gravity>(gfx::GRAVITY_LEFT | gfx::GRAVITY_TOP)),
      multiline_(false) {
  gravity_ = gfx::GRAVITY_CENTER;
}

void TextView::set_text(const std::wstring& str) {
  text_ = str;
  // TODO: what to do here?!
  lines_to_render_.clear();
}

void TextView::set_text(std::wstring&& str) {
  text_ = std::move(str);
  // TODO: what to do here?!
  lines_to_render_.clear();
}

void TextView::set_gravity(gfx::Gravity gravity) noexcept {
  gravity_ = gravity;
}

void TextView::set_multiline(bool multiline) noexcept {
  multiline_ = multiline;
}

int TextView::text_len_() const {
  return text_.size();
}

}  // namespace cursedui::view
