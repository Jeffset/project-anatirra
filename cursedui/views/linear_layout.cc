// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/linear_layout.hpp"

#include "base/debug/debug.hpp"
#include "base/util.hpp"

#include <cmath>
#include <utility>

namespace cursedui::view {

namespace {

constexpr auto oriented_dim_table = std::array{&gfx::Size::width, &gfx::Size::height};
constexpr auto orthogonal_dim_table = std::array{&gfx::Size::height, &gfx::Size::width};

}  // namespace

gfx::Size LinearLayout::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  gfx::Size this_size = View::on_measure(width_spec, height_spec);
  match_parent_children_tmp_.clear();

  const bool is_horizontal = orientation_ == HORIZONTAL;
  const MeasureSpec oriented_spec = is_horizontal ? width_spec : height_spec;

  const auto oriented_dim = oriented_dim_table[orientation_];
  const auto orthogonal_dim = orthogonal_dim_table[orientation_];

  const auto [use_weights, total_oriented_dim] =
      std::visit(base::overloaded{
                     [](const MeasureExactly& spec) { return std::pair(true, spec.dim); },
                     [](auto) { return std::pair(false, -1); },
                 },
                 oriented_spec);

  gfx::dim_t weightless_children_size = 0;
  float child_summary_weight = 0.f;
  int weighted_child_count = 0;

  for (const auto& child : *this) {
    const auto* lp = static_cast<LayoutParams*>(child->layout_params().get());
    const auto wls = lp->width_layout_spec();
    const auto hls = lp->height_layout_spec();

    if (base::holds_alternative<LayoutMatchParent>(wls) ||
        base::holds_alternative<LayoutMatchParent>(hls)) {
      // We layout match-parent views again after all the rest.
      match_parent_children_tmp_.push_back(child.get());
    }

    if (!use_weights || !lp->weight()) {
      child->measure(make_measure_spec(wls, width_spec),
                     make_measure_spec(hls, height_spec));
      weightless_children_size += child->measured_size().*oriented_dim;

      this_size.*orthogonal_dim =
          std::max(this_size.*orthogonal_dim, child->measured_size().*orthogonal_dim);
    } else {
      child_summary_weight += lp->weight().value();
      ++weighted_child_count;
    }
  }

  if (use_weights) {
    // NOTE: here we can detect, that there's not enough width for us.
    const auto width_for_weighting =
        std::max(total_oriented_dim - weightless_children_size, 0);
    gfx::dim_t distributed_dim = 0;
    int weighted_child_num = 1;

    for (const auto& child : *this) {
      const auto* lp = static_cast<LayoutParams*>(child->layout_params().get());

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
      } else {
        child->measure(make_measure_spec(lp->width_layout_spec(), width_spec),
                       MeasureExactly{{dim}});
      }

      this_size.*orthogonal_dim =
          std::max(this_size.*orthogonal_dim, child->measured_size().*orthogonal_dim);
      ++weighted_child_num;
    }

    this_size.*oriented_dim = total_oriented_dim;
  } else {
    this_size.*oriented_dim = weightless_children_size;
  }

  for (auto* child : match_parent_children_tmp_) {
    const auto* lp = static_cast<LayoutParams*>(child->layout_params().get());
    const auto wls = lp->width_layout_spec();
    const auto hls = lp->height_layout_spec();

    const MeasureSpec child_width_spec =
        is_horizontal && lp->weight() ? MeasureExactly{{child->measured_size().width}}
                                      : base::holds_alternative<LayoutMatchParent>(wls)
                                            ? MeasureExactly{{this_size.width}}
                                            : make_measure_spec(wls, width_spec);
    const MeasureSpec child_height_spec =
        !is_horizontal && lp->weight() ? MeasureExactly{{child->measured_size().height}}
                                       : base::holds_alternative<LayoutMatchParent>(hls)
                                             ? MeasureExactly{{this_size.height}}
                                             : make_measure_spec(hls, height_spec);

    child->measure(child_width_spec, child_height_spec);
  }

  this_size.width = fix_measure(this_size.width, width_spec);
  this_size.height = fix_measure(this_size.height, height_spec);
  return this_size;
}

void LinearLayout::on_layout() {
  const bool is_horizontal = orientation_ == HORIZONTAL;
  const auto size = inner_bounds().size();

  auto position = inner_bounds().position();
  for (const auto& child : *this) {
    const auto available_size =
        is_horizontal ? gfx::Size{child->measured_size().width, size.height}
                      : gfx::Size{size.width, child->measured_size().height};
    child->layout(gfx::gravitated_rect(gfx::rect_from(position, available_size),
                                       child->measured_size(),
                                       child->layout_params().get()->gravity()));
    if (is_horizontal)
      position.x += available_size.width;
    else
      position.y += available_size.height;
  }
}

void LinearLayout::propagate_needs_layout_mark(const View* child) {
  using namespace base::operators;
  ASSERT(child->get_parent() == this);
  auto* lp = static_cast<LayoutParams*>(child->layout_params().get());
  if (child->needs_layout().has(NeedsLayout::WIDTH) &&
      base::holds_alternative<LayoutWrapContent>(lp->width_layout_spec()) &&
      !(lp->weight() && orientation_ == HORIZONTAL)) {
    mark_needs_layout(NeedsLayout::WIDTH | NeedsLayout::CONTENT);
  }
  if (child->needs_layout().has(NeedsLayout::HEIGHT) &&
      base::holds_alternative<LayoutWrapContent>(lp->height_layout_spec()) &&
      !(lp->weight() && orientation_ == VERTICAL)) {
    mark_needs_layout(NeedsLayout::HEIGHT | NeedsLayout::CONTENT);
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
  mark_needs_layout(NeedsLayout::SIZE);
}

LinearLayout::LayoutParams::LayoutParams(LayoutSpec width,
                                         LayoutSpec height,
                                         base::EnumFlags<gfx::Gravity> gravity) noexcept
    : view::LayoutParams(width, height, gravity) {}

LinearLayout::LayoutParams::LayoutParams(LayoutSpec width,
                                         LayoutSpec height,
                                         float weight,
                                         base::EnumFlags<gfx::Gravity> gravity) noexcept
    : view::LayoutParams(width, height, gravity), weight_(weight) {
  ASSERT(weight > 0.0001f);
}

std::string_view LinearLayout::LayoutParams::tag() const noexcept {
  return TAG;
}

const char* LinearLayout::LayoutParams::TAG = "LinearLayout";

void LinearLayout::LayoutParams::set_weight(float weight) {
  ASSERT(weight > 0.0001f);
  if (weight_ == weight) {
    return;
  }
  weight_ = weight;
  mark_needs_layout(NeedsLayout::SIZE);
}

void LinearLayout::LayoutParams::set_no_weight() {
  if (!weight_.has_value()) {
    return;
  }
  weight_.reset();
  mark_needs_layout(NeedsLayout::SIZE);
}

}  // namespace cursedui::view
