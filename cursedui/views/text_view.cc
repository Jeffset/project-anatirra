// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/text_view.hpp"

#include "avada/input.hpp"
#include "base/util.hpp"
#include "cursedui/canvas.hpp"
#include "cursedui/drawable.hpp"

#include <iostream>
#include <limits>
#include <string_view>

namespace cursedui::view {

namespace {

std::wstring_view create_view(std::wstring::const_iterator begin,
                              std::wstring::const_iterator end) {
  return {&*begin, static_cast<size_t>(end - begin)};
}

}  // namespace

gfx::Size TextView::measure_text(gfx::dim_t max_width,
                                 gfx::dim_t max_height) const noexcept {
  if (!multiline_) {
    auto newline_pos = text_.find_first_of('\n');
    auto line_len = newline_pos == std::wstring::npos ? text_.size() : newline_pos;
    return {std::min(max_width, static_cast<gfx::dim_t>(line_len)),
            std::min(1, max_height)};
  }

  auto end = std::cend(text_);
  auto line_start = std::cbegin(text_);
  auto it = line_start;
  auto width = 0;
  auto height = 1;
  auto unfinished_line = true;

  for (;;) {
    auto current_width = it - line_start;
    width = std::max(width, (gfx::dim_t)current_width);

    if (it == end) {
      unfinished_line = false;
      break;
    }

    if (*it == '\n') {
      line_start = it + 1;
      ++height;
    } else if (current_width >= max_width) {
      line_start = it;
      width = max_width;
      ++height;
    }

    if (height > max_height) {
      height = max_height;
      unfinished_line = false;
      break;
    }

    ++it;
  }

  if (unfinished_line)
    ++height;

  return {width, height};
}

gfx::Size TextView::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  gfx::Size size{};
  if (base::holds_alternative<MeasureExactly>(width_spec) &&
      base::holds_alternative<MeasureExactly>(height_spec)) {
    size = {std::get<MeasureExactly>(width_spec).dim,
            std::get<MeasureExactly>(height_spec).dim};
  } else {
    constexpr auto max_dim_generator = base::overloaded{
        [](MeasureUnlimited) { return std::numeric_limits<gfx::dim_t>::max(); },
        [](MeasureSpecified specified) { return specified.dim; }};

    auto max_width = std::visit(max_dim_generator, width_spec);
    auto max_height = std::visit(max_dim_generator, height_spec);

    size = measure_text(max_width, max_height);
    size.width = std::visit(measure_exactly_or(size.width), width_spec);
    size.height = std::visit(measure_exactly_or(size.height), height_spec);
  }
  return size;
}

void TextView::on_layout() {
  auto bounds = inner_bounds();
  auto max_width = bounds.width();
  auto max_height = multiline_ ? bounds.height() : 1;

  auto end = std::cend(text_);
  auto line_start = std::cbegin(text_);
  auto it = line_start;
  auto line_count = 1;
  auto unfinished_line = true;

  auto width = 0;

  lines_to_render_.clear();
  ellipsize_ = false;
  while (true) {
    if (it == end) {
      lines_to_render_.push_back(create_view(line_start, it));
      unfinished_line = false;
      break;
    }

    if (*it == '\n') {
      lines_to_render_.push_back(create_view(line_start, it));
      line_start = it + 1;
      ++line_count;
    } else if ((it - line_start) >= max_width) {
      lines_to_render_.push_back(create_view(line_start, it));
      line_start = it;
      ++line_count;
    }

    if (line_count > max_height) {
      line_count = max_height;
      ellipsize_ = true;
      unfinished_line = false;
      break;
    }

    ++it;
  }

  if (unfinished_line) {
    lines_to_render_.push_back(create_view(line_start, it));
    ++line_count;
  }

  for (auto& line : lines_to_render_)
    width = std::max(width, static_cast<gfx::dim_t>(line.size()));

  auto text_block_size = gfx::Size{std::min(width, max_width), line_count};
  if (ellipsize_ && !lines_to_render_.empty()) {
    // this is for '...' symbol.
    auto& last_line = lines_to_render_.back();
    if (static_cast<int>(last_line.size()) == max_width)
      lines_to_render_.back().remove_suffix(1);
  }
  text_pos_ = gfx::gravitated_rect(bounds, text_block_size, gravity_).position();
}

void TextView::on_key_event(const avada::input::KeyboardEvent& event) {
  std::visit(base::overloaded{
                 [this](wchar_t key) { set_text(text_ + key); },
                 [this](avada::input::KeyboardKey key) {
                   if (key == avada::input::KeyboardKey::BACKSPACE) {
                     set_text(text_.substr(0, text_.size() - 1));
                   }
                 },
             },
             event.key);
}

void TextView::on_draw(paint::Canvas& canvas) {
  View::on_draw(canvas);

  const paint::Pen pen{text_color_, avada::render::Colors::TRANSPARENT};
  auto pos = text_pos_;
  for (auto& line : lines_to_render_) {
    canvas.draw(line, pos, pen);
    ++pos.y;
  }
  if (ellipsize_) {
    --pos.y;
    pos.x += lines_to_render_.back().size();
    canvas.draw(L'…', pos, pen);
  }
}

void TextView::on_focus_changed(bool focused) {
  if (focused) {
    border().set_style(BorderDrawable::Style::DOUBLE);
  } else {
    border().set_style(BorderDrawable::Style::SINGLE);
  }
  mark_needs_paint();
}

TextView::TextView()
    : gravity_(gfx::Gravity::CENTER),
      multiline_(false),
      text_color_(avada::render::SystemColor::DEFAULT) {}

void TextView::set_text(std::wstring str) {
  using namespace base::operators;
  if (text_ == str)
    return;
  lines_to_render_.clear();
  text_ = std::move(str);
  auto size_mark = multiline_ ? NeedsLayout::SIZE : NeedsLayout::WIDTH;
  mark_needs_layout(size_mark | NeedsLayout::CONTENT);
  mark_needs_paint();
}

void TextView::set_gravity(base::EnumFlags<gfx::Gravity> gravity) noexcept {
  if (gravity_ == gravity)
    return;
  gravity_ = gravity;
  mark_needs_layout(NeedsLayout::CONTENT);
}

void TextView::set_multiline(bool multiline) noexcept {
  if (multiline_ == multiline)
    return;
  multiline_ = multiline;
  mark_needs_layout(NeedsLayout::SIZE);
  // do not mark for paint here, it'll be marked if layout actually changes.
}

void TextView::set_text_color(avada::render::Color color) noexcept {
  if (text_color_ == color)
    return;
  text_color_ = color;
  mark_needs_paint();
}

}  // namespace cursedui::view
