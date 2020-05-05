// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/linear_layout.hpp"

#include "base/debug.hpp"
#include "base/util.hpp"

#include <cmath>
#include <utility>

namespace cursedui::view {

void LinearLayout::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  const bool is_horizontal = orientation_ == HORIZONTAL;
  const MeasureSpec oriented_spec = is_horizontal ? width_spec : height_spec;
  const MeasureSpec orthogonal_spec = is_horizontal ? height_spec : width_spec;

  const auto [oriented_dim, orthogonal_dim] =
      is_horizontal ? std::make_pair(&gfx::Size::width, &gfx::Size::height)
                    : std::make_pair(&gfx::Size::height, &gfx::Size::width);

  const auto count = child_count();
  const auto [use_weights, total_oriented_dim] = std::visit(
      base::overloaded{
          [](const MeasureSpecified& spec) { return std::pair(true, spec.dim); },
          [](MeasureUnlimited) { return std::pair(false, -1); }},
      oriented_spec);

  gfx::Size this_size{};
  this_size.*oriented_dim =
      std::visit(base::overloaded{
                     [](const MeasureExactly& exactly) { return exactly.dim; },
                     [](const auto&) { return 0; },
                 },
                 orthogonal_spec);

  gfx::dim_t weightless_children_size = 0;
  float child_summary_weight = 0.f;
  int weighted_child_count = 0;

  for (int i = 0; i < count; ++i) {
    auto* child = get_child(i);
    const auto* lp = (LayoutParams*)child->layout_params().get();

    if (!use_weights || !lp->weight()) {
      child->measure(make_measure_spec(lp->width_layout_spec(), width_spec),
                     make_measure_spec(lp->height_layout_spec(), height_spec));
      child->layout_propagation_mask =
          make_layout_propagation_mask(lp->width_layout_spec(), width_spec,
                                       NEEDS_LAYOUT_WIDTH) |
          make_layout_propagation_mask(lp->height_layout_spec(), height_spec,
                                       NEEDS_LAYOUT_HEIGHT);
      weightless_children_size += child->measured_size().*oriented_dim;

      this_size.*orthogonal_dim =
          std::max(this_size.*orthogonal_dim, child->measured_size().*orthogonal_dim);
    } else {
      child_summary_weight += lp->weight().value();
      ++weighted_child_count;
    }
  }

  if (use_weights) {
    // TODO: here we can detect, that there's not enough width for us.
    const gfx::dim_t width_for_weighting =
        std::max(total_oriented_dim - weightless_children_size, 0);
    gfx::dim_t distributed_dim = 0;
    int weighted_child_num = 1;

    for (int i = 0; i < count; ++i) {
      auto* child = get_child(i);
      const auto* lp = (LayoutParams*)child->layout_params().get();

      if (!lp->weight()) {
        continue;
      }

      const float normalized_weight = lp->weight().value() / child_summary_weight;
      gfx::dim_t dim =
          weighted_child_num == weighted_child_count
              ? width_for_weighting - distributed_dim
              : (gfx::dim_t)std::round(width_for_weighting * normalized_weight);

      distributed_dim += dim;

      if (is_horizontal) {
        child->measure(MeasureExactly{{dim}},
                       make_measure_spec(lp->height_layout_spec(), height_spec));
        child->layout_propagation_mask =
            NEEDS_LAYOUT_WIDTH |
            make_layout_propagation_mask(lp->height_layout_spec(), height_spec,
                                         NEEDS_LAYOUT_HEIGHT);
      } else {
        child->measure(make_measure_spec(lp->width_layout_spec(), width_spec),
                       MeasureExactly{{dim}});
        child->layout_propagation_mask =
            make_layout_propagation_mask(lp->width_layout_spec(), width_spec,
                                         NEEDS_LAYOUT_WIDTH) |
            NEEDS_LAYOUT_HEIGHT;
      }

      this_size.*orthogonal_dim =
          std::max(this_size.*orthogonal_dim, child->measured_size().*orthogonal_dim);
      ++weighted_child_num;
    }

    this_size.*oriented_dim = total_oriented_dim;
  } else {
    this_size.*oriented_dim = weightless_children_size;
  }

  set_measured_size(this_size);
}

void LinearLayout::on_layout() {
  const bool is_horizontal = orientation_ == HORIZONTAL;
  const auto count = child_count();
  auto position = inner_bounds().position();
  for (int i = 0; i < count; ++i) {
    auto* child = get_child(i);
    const auto child_size = child->measured_size();
    child->layout(gfx::rect_from(position, child_size));
    if (is_horizontal)
      position.x += child_size.width;
    else
      position.y += child_size.height;
  }
}

std::unique_ptr<view::LayoutParams> LinearLayout::create_layout_params() const noexcept {
  return std::make_unique<LayoutParams>(LayoutWrapContent{}, LayoutWrapContent{});
}

bool LinearLayout::check_layout_params(view::LayoutParams* params) const noexcept {
  return params->tag() == LayoutParams::TAG;
}

LinearLayout::~LinearLayout() noexcept = default;

LinearLayout::LinearLayout() noexcept : orientation_(HORIZONTAL) {}

void LinearLayout::set_orientation(LinearLayout::Orientation orientation) noexcept {
  if (orientation_ == orientation)
    return;
  orientation_ = orientation;
}

LinearLayout::LayoutParams::LayoutParams(const LayoutSpec& width,
                                         const LayoutSpec& height)
    : view::LayoutParams(width, height) {}

std::string_view LinearLayout::LayoutParams::tag() const noexcept {
  return TAG;
}

const char* LinearLayout::LayoutParams::TAG = "LinearLayout";

void LinearLayout::LayoutParams::set_weight(float weight) {
  ASSERT(weight > 0.f);
  weight_ = weight;
}

void LinearLayout::LayoutParams::set_no_weight() {
  weight_.reset();
}

}  // namespace cursedui::view
