/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cursedui/canvas.hpp"

#include "avada/buffer.hpp"
#include "avada/color.hpp"

#include <algorithm>
#include <variant>

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
  if (pen.fg_blend_mode == BlendMode::BLEND && is_blank_char(data) && 
      !std::holds_alternative<avada::render::SystemColor>(pen.bg_color)) {
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

}  // namespace

Canvas::Canvas(avada::render::Buffer& buffer) noexcept : buffer_(buffer) {}

template <typename Char>
void Canvas::draw(std::basic_string_view<Char> str,
                  gfx::Point start,
                  const Pen& pen) noexcept {
  auto rect = gfx::rect_from(start, {static_cast<gfx::dim_t>(str.size()), 1});
  auto clipped = apply_clip(rect);
  if (clipped.empty())
    return;

  for (auto& clipped_rect : clipped) {
    auto clipped_str = str;
    clipped_str.remove_prefix(clipped_rect.left - rect.left);
    auto str_iter = std::begin(clipped_str);
    for (auto j = clipped_rect.left; j <= clipped_rect.right; ++j)
      paint_cell(buffer_(clipped_rect.top, j), *str_iter++, pen);
  }
}

template <typename Char>
void Canvas::draw(Char c, gfx::Point position, const Pen& pen) noexcept {
  if (UNLIKELY(position.x < 0 || position.x >= buffer_.columns()))
    return;
  if (UNLIKELY(position.y < 0 || position.y >= buffer_.rows()))
    return;
  if (!clip_stack_.empty() && !clip_stack_.top().clip(position))
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
  auto clipped = apply_clip(rect);
  if (clipped.empty())
    return;

  if (direction == Direction::HORIZONTAL)
    for (auto& region_part : clipped)
      for (auto j = region_part.left; j <= region_part.right; ++j)
        paint_cell(buffer_(region_part.top, j), c, pen);
  else
    for (auto& region_part : clipped)
      for (auto i = region_part.top; i <= region_part.bottom; ++i)
        paint_cell(buffer_(i, region_part.left), c, pen);
}

template <typename Char>
void Canvas::fill(Char c, gfx::Rect rect, const Pen& pen) noexcept {
  auto clipped = apply_clip(rect);
  if (clipped.empty())
    return;

  for (auto& region_part : clipped)
    for (auto i = region_part.top; i <= region_part.bottom; ++i)
      for (auto j = region_part.left; j <= region_part.right; ++j)
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
  auto clipped = apply_clip(rect);
  if (clipped.empty())
    return;

  // FIXME: invalid start of i_s, j_s;
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

Canvas::ScopedClipHandle Canvas::push_clip(const gfx::Rect& rect) noexcept {
  if (UNLIKELY(clip_stack_.empty())) {
    clip_stack_.emplace(rect);
    return {*this, !rect.has_area()};
  }
  auto new_clip = clip_stack_.top().clip(rect);
  clip_stack_.push(new_clip);
  return {*this, new_clip.empty()};
}

Canvas::ScopedClipHandle Canvas::push_clip(const Region& region) noexcept {
  if (UNLIKELY(clip_stack_.empty())) {
    clip_stack_.push(region);
    return {*this, region.empty()};
  }
  auto new_clip = clip_stack_.top().clip(region);
  clip_stack_.push(new_clip);
  return {*this, new_clip.empty()};
}

void Canvas::pop_clip() noexcept {
  clip_stack_.pop();
}

paint::Region Canvas::apply_clip(const gfx::Rect& rect) const noexcept {
  auto clipped = clip_to_buffer(rect);

  if (UNLIKELY(clip_stack_.empty())) {
    return paint::Region{clipped};
  }

  return clip_stack_.top().clip(clipped);
}

gfx::Rect Canvas::clip_to_buffer(const gfx::Rect& rect) const noexcept {
  gfx::Rect clipped;
  clipped.left = std::max(0, rect.left);
  clipped.right = std::min(buffer_.columns(), rect.right);
  clipped.top = std::max(0, rect.top);
  clipped.bottom = std::min(buffer_.rows(), rect.bottom);
  return clipped;
}

Canvas::ScopedClipHandle::~ScopedClipHandle() {
  self_.pop_clip();
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
