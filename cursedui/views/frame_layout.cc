// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/frame_layout.hpp"

#include "base/util.hpp"
#include "cursedui/view_specs.hpp"

namespace cursedui::view {

gfx::Size FrameLayout::on_measure(MeasureSpec width_spec,
                                  MeasureSpec height_spec,
                                  bool update_layout_masks) {
  gfx::Size size{};
  for (const auto& child : *this) {
    auto* lp = child->layout_params().get();
    child->measure(make_measure_spec(lp->width_layout_spec(), width_spec),
                   make_measure_spec(lp->height_layout_spec(), height_spec));
    if (update_layout_masks) {
      layout_propagation_masks_[child.get()] =
          make_layout_propagation_mask(lp->width_layout_spec(), width_spec,
                                       NeedsLayout::WIDTH) |
          make_layout_propagation_mask(lp->height_layout_spec(), height_spec,
                                       NeedsLayout::HEIGHT);
    }
    size = gfx::max(size, child->measured_size());
  }
  return size;
}

void FrameLayout::on_layout() {
  for (const auto& child : *this) {
    auto* lp = child->layout_params().get();
    auto rect =
        gfx::gravitated_rect(inner_bounds(), child->measured_size(), lp->gravity());
    child->layout(rect);
  }
}

}  // namespace cursedui::view
