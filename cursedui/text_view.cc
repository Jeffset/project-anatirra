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
  auto [width, height] = bounds.size();
  auto text_len = text_len_();
  if (multiline_) {
    ellipsize_ = false;
    lines_to_render_.clear();
    auto begin = std::begin(text_);
    auto end = std::end(text_);
    auto it = begin;
    auto line_start = begin;
    int line_count = 0;
    while (it != end) {
      if (line_count >= height) {
        ellipsize_ = true;
        break;
      }
      auto ch = *it;
      if (ch == L'\n' || (it - line_start + 1) >= width) {
        std::wstring_view line{text_};
        line.remove_prefix(line_start - begin);
        line.remove_suffix(end - ++it);
        if (ch == '\n')
          line.remove_suffix(1);
        lines_to_render_.push_back(line);
        ++line_count;
        line_start = it;
      } else {
        ++it;
      }
    }

    auto text_size = gfx::Size{std::min(text_len, width), line_count};
    if (ellipsize_) {
      // this is for '...' symbol.
      auto& last_line = lines_to_render_.back();
      if (last_line.size() == (size_t)width)
        lines_to_render_.back().remove_suffix(1);
    }
    text_pos_ = gfx::gravitated_rect(bounds, text_size, gravity_).position();
  } else {
    // FIXME: strip text of '\n'
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
  text_color_ = palette.obtain_color(text_color_descr_);
}

void TextView::on_key_event(const input::KeyEvent& event) {
  if (event.key_char) {
    text_ += event.key_char.value();
  }
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
      multiline_(false),
      text_color_descr_(render::SystemColor::WHITE) {
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

void TextView::set_text_color(render::ColorDescr descr) noexcept {
  text_color_descr_ = descr;
}

int TextView::text_len_() const {
  return text_.size();
}

}  // namespace cursedui::view
