// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/scroll_view.hpp"

#include "avada/input.hpp"
#include "base/util.hpp"
#include "cursedui/view_specs.hpp"

#include <cmath>

namespace cursedui::view {

ScrollView::ScrollView() noexcept
    : direction_{ScrollDirection::VERTICAL},
      scroll_offset_{0, 0},
      max_scroll_offset_{0, 0} {}

ScrollView::~ScrollView() noexcept = default;

void ScrollView::set_direction(
    base::EnumFlags<ScrollView::ScrollDirection> direction) noexcept {
  using namespace base::operators;
  if (direction_ == direction)
    return;

  direction_ = direction;
  mark_needs_layout(NeedsLayout::SIZE | NeedsLayout::CONTENT);
}

gfx::Size ScrollView::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  const auto inner_width_spec =
      direction_.has(ScrollDirection::HORIZONTAL) ? MeasureUnlimited{} : width_spec;
  const auto inner_height_spec =
      direction_.has(ScrollDirection::VERTICAL) ? MeasureUnlimited{} : height_spec;

  auto size = FrameLayout::on_measure(inner_width_spec, inner_height_spec);

  gfx::Size group_size{
      fix_measure(size.width, width_spec),
      fix_measure(size.height, height_spec),
  };

  max_scroll_offset_ = {
      std::max(0, size.width - group_size.width),
      std::max(0, size.height - group_size.height),
  };
  scroll_offset_ = {
      std::min(scroll_offset_.width, max_scroll_offset_.width),
      std::min(scroll_offset_.height, max_scroll_offset_.height),
  };

  LOG() << scroll_offset_ << "/" << max_scroll_offset_;

  return group_size;
}

void ScrollView::on_layout() {
  const auto bounds = inner_bounds();
  for (const auto& child : *this) {
    auto* lp = child->layout_params().get();
    auto gravity = lp->gravity();
    const auto child_size = child->measured_size();
    if (direction_.has(ScrollDirection::HORIZONTAL) &&
        child_size.width > bounds.width()) {
      gravity.remove(gfx::Gravity::RIGHT);
    }
    if (direction_.has(ScrollDirection::VERTICAL) &&
        child_size.height > bounds.height()) {
      gravity.remove(gfx::Gravity::BOTTOM);
    }
    auto rect = gfx::gravitated_rect(bounds, child_size, gravity);
    rect = gfx::move(rect, {-scroll_offset_.width, -scroll_offset_.height});
    child->layout(rect);
  }
}

bool ScrollView::intercept_mouse_event(const avada::input::MouseEvent& event) {
  return base::holds_alternative<avada::input::MouseEvent::Scroll>(event.data);
}

void ScrollView::on_mouse_event(const avada::input::MouseEvent& event) {
  using namespace avada::input;
  std::visit(base::overloaded{
                 [this](MouseEvent::Scroll scroll) {
                   if (scroll == MouseEvent::Scroll::UP) {
                     scroll_by(0, -1);
                   } else {
                     scroll_by(0, +1);
                   }
                 },
                 [](auto) {},
             },
             event.data);
}

namespace {
using namespace avada::render;

const paint::Pen scroll_area_pen{Colors::WHITE, ColorRGB{255, 255, 00, 64}};
const paint::Pen scroll_bar_pen{Colors::WHITE, ColorRGB{255, 255, 255, 64}};

}  // namespace

void ScrollView::on_draw(paint::Canvas& canvas) {
  FrameLayout::on_draw(canvas);
  if (max_scroll_offset_.height != 0) {
    const auto bounds = inner_bounds();
    const auto scroll_bar_area = bounds.height();
    auto bar_len = std::max(1, scroll_bar_area * bounds.height() /
                                   (bounds.height() + max_scroll_offset_.height));
    auto pos =
        (scroll_bar_area - bar_len) * scroll_offset_.height / max_scroll_offset_.height;

    canvas.draw_line(' ', {bounds.right, bounds.top}, paint::Direction::VERTICAL,
                     scroll_bar_area, scroll_area_pen);
    canvas.draw_line(' ', {bounds.right, bounds.top + pos}, paint::Direction::VERTICAL,
                     bar_len, scroll_bar_pen);
  }
}

void ScrollView::scroll_by(gfx::dim_t dx, gfx::dim_t dy) noexcept {
  const auto old_scroll_offset = scroll_offset_;
  scroll_offset_ = {
      base::clamp(scroll_offset_.width + dx, 0, max_scroll_offset_.width),
      base::clamp(scroll_offset_.height + dy, 0, max_scroll_offset_.height),
  };

  if (old_scroll_offset == scroll_offset_)
    return;

  mark_needs_layout(NeedsLayout::CONTENT);
}

}  // namespace cursedui::view
