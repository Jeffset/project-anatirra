// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."
#include "cursedui/canvas.hpp"

#include "avada/buffer.hpp"
#include "base/debug/debug.hpp"

#include <algorithm>

namespace cursedui::paint {

namespace {

template <class Char>
bool is_blank_char(Char c) {
  return c == ' ';
}

inline avada::render::Color blend(avada::render::Color src,
                                  avada::render::Color dst,
                                  BlendMode mode) {
  if (mode == BlendMode::BLEND) {
    return avada::render::alpha_blend(src, dst);
  }
  return src;
}

template <class Char>
void paint_cell(avada::render::Buffer::Cell& cell, Char data, const Pen& pen) {
  if (pen.fg_blend_mode == BlendMode::BLEND && is_blank_char(data)) {
    // if blend mode is on and data is blank: no data overwriting and we blend fg with bg
    // NOTE: thus, fg color is ignored (is this right?)
    auto color = blend(pen.bg_color, cell.fg_color(), pen.fg_blend_mode);
    cell.set_fg_color(color);
  } else {
    // if foreground should be blitted or data is not blank: overwrite data.
    cell.set_data(data);
    cell.set_fg_color(blend(pen.fg_color, cell.fg_color(), pen.fg_blend_mode));
  }

  // Always draw background over background.
  cell.set_bg_color(blend(pen.bg_color, cell.bg_color(), pen.bg_blend_mode));

  cell.set_attributes(pen.render_attributes_blend_mode == BlendMode::BLEND
                          ? cell.attributes() | pen.render_attributes
                          : pen.render_attributes);
}

void do_clip(gfx::Rect& rect, const gfx::Rect& clip) noexcept {
  rect.left = std::max(clip.left, rect.left);
  rect.right = std::min(clip.right, rect.right);
  rect.top = std::max(clip.top, rect.top);
  rect.bottom = std::min(clip.bottom, rect.bottom);
}

}  // namespace

Canvas::Canvas(avada::render::Buffer& buffer) noexcept : buffer_(buffer) {}

template <typename Char>
void Canvas::draw(std::basic_string_view<Char> str,
                  gfx::Point start,
                  const Pen& pen) noexcept {
  auto rect = gfx::rect_from(start, {static_cast<gfx::dim_t>(str.size()), 1});
  auto clipped_rect = rect;

  if (!apply_clip(clipped_rect))
    return;

  str.remove_prefix(clipped_rect.left - rect.left);

  auto str_iter = std::begin(str);
  for (auto j = clipped_rect.left; j <= clipped_rect.right; ++j)
    paint_cell(buffer_(clipped_rect.top, j), *str_iter++, pen);
}

template <typename Char>
void Canvas::draw(Char c, gfx::Point position, const Pen& pen) noexcept {
  if (clip_.has_value() && !clip_.value().contains(position))
    return;
  if (position.x < 0 || position.x >= buffer_.columns())
    return;
  if (position.y < 0 || position.y >= buffer_.rows())
    return;
  paint_cell(buffer_(position.y, position.x), c, pen);
}

template <typename Char>
void Canvas::draw_line(Char c,
                       gfx::Point start,
                       Direction direction,
                       gfx::dim_t count,
                       const Pen& pen) noexcept {
  auto rect = direction == Direction::HORIZONTAL ? gfx::rect_from(start, {count, 1})
                                                 : gfx::rect_from(start, {1, count});
  if (!apply_clip(rect))
    return;
  if (direction == Direction::HORIZONTAL)
    for (auto j = rect.left; j <= rect.right; ++j)
      paint_cell(buffer_(rect.top, j), c, pen);
  else
    for (auto i = rect.top; i <= rect.bottom; ++i)
      paint_cell(buffer_(i, rect.left), c, pen);
}

template <typename Char>
void Canvas::fill(Char c, gfx::Rect rect, const Pen& pen) noexcept {
  if (!apply_clip(rect))
    return;
  for (auto i = rect.top; i <= rect.bottom; ++i)
    for (auto j = rect.left; j <= rect.right; ++j)
      paint_cell(buffer_(i, j), c, pen);
}

void Canvas::draw(const avada::render::Buffer& buffer,
                  gfx::Point target_position,
                  BlendMode blend_mode) noexcept {
  draw(buffer, gfx::rect_from({0, 0}, {buffer.columns(), buffer.rows()}), target_position,
       blend_mode);
}

void Canvas::draw(const avada::render::Buffer& buffer,
                  gfx::Rect source_rect,
                  gfx::Point target_position,
                  const BlendMode blend_mode) noexcept {
  auto rect = gfx::rect_from(target_position, source_rect.size());
  if (!apply_clip(rect))
    return;

  for (auto i = rect.top, i_s = 0; i <= rect.bottom; ++i, ++i_s)
    for (auto j = rect.left, j_s = 0; j <= rect.right; ++j, ++j_s) {
      auto& dst = buffer_(i, j);
      const auto& src = buffer(i_s, j_s);
      if (blend_mode == BlendMode::BLEND)
        dst.blend(src);
      else {
        dst.assign(src);
      }
    }
}

void Canvas::set_clip(gfx::Rect clip) noexcept {
  clip_ = clip;
}

void Canvas::reset_clip() noexcept {
  clip_.reset();
}

bool Canvas::apply_clip(gfx::Rect& rect) const noexcept {
  if (clip_.has_value())
    do_clip(rect, clip_.value());
  clip_to_buffer(rect);
  return rect.has_area();
}

void Canvas::clip_to_buffer(gfx::Rect& rect) const noexcept {
  do_clip(rect, gfx::rect_from({0, 0}, {buffer_.columns(), buffer_.rows()}));
}

// Since we define template functions out-of-line inside a *.cc file, we need to
// instantiate them explicitly (for char and wchar_t);

template void Canvas::draw<char>(std::basic_string_view<char> str,
                                 gfx::Point start,
                                 const Pen& pen);
template void Canvas::draw<wchar_t>(std::basic_string_view<wchar_t> str,
                                    gfx::Point start,
                                    const Pen& pen);

template void Canvas::draw<char>(char c, gfx::Point position, const Pen& pen);
template void Canvas::draw<wchar_t>(wchar_t c, gfx::Point position, const Pen& pen);

template void Canvas::draw_line<char>(char c,
                                      gfx::Point start,
                                      Direction direction,
                                      gfx::dim_t count,
                                      const Pen& pen);
template void Canvas::draw_line<wchar_t>(wchar_t c,
                                         gfx::Point start,
                                         Direction direction,
                                         gfx::dim_t count,
                                         const Pen& pen);

template void Canvas::fill<char>(char c, gfx::Rect rect, const Pen& pen);
template void Canvas::fill<wchar_t>(wchar_t c, gfx::Rect rect, const Pen& pen);

}  // namespace cursedui::paint
