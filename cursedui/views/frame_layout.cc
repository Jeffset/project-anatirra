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

#include "cursedui/views/frame_layout.hpp"

#include "base/util.hpp"
#include "cursedui/view_specs.hpp"

namespace cursedui::view {

gfx::Size FrameLayout::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  gfx::Size size = View::on_measure(width_spec, height_spec);
  match_parent_children_tmp_.clear();

  for (const auto& child : *this) {
    const auto* lp = child->layout_params().get();
    const auto wls = lp->width_layout_spec();
    const auto hls = lp->height_layout_spec();

    if (base::holds_alternative<LayoutMatchParent>(wls) ||
        base::holds_alternative<LayoutMatchParent>(hls)) {
      // We layout match-parent views again after all the rest.
      match_parent_children_tmp_.push_back(child.get());
    }

    child->measure(make_measure_spec(wls, width_spec),
                   make_measure_spec(hls, height_spec));
    size = gfx::max(size, child->measured_size());
  }

  for (auto* child : match_parent_children_tmp_) {
    const auto* lp = child->layout_params().get();
    const auto wls = lp->width_layout_spec();
    const auto hls = lp->height_layout_spec();
    const MeasureSpec child_width_spec = base::holds_alternative<LayoutMatchParent>(wls)
                                             ? MeasureExactly{{size.width}}
                                             : make_measure_spec(wls, width_spec);
    const MeasureSpec child_height_spec = base::holds_alternative<LayoutMatchParent>(hls)
                                              ? MeasureExactly{{size.height}}
                                              : make_measure_spec(hls, height_spec);

    child->measure(child_width_spec, child_height_spec);
  }
  size.width = fix_measure(size.width, width_spec);
  size.height = fix_measure(size.height, height_spec);
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

void FrameLayout::propagate_needs_layout_mark(const View* child) {
  using namespace base::operators;
  ASSERT(child->get_parent() == this);
  auto* lp = child->layout_params().get();
  if (child->needs_layout().has(NeedsLayout::WIDTH) &&
      base::holds_alternative<LayoutWrapContent>(lp->width_layout_spec())) {
    mark_needs_layout(NeedsLayout::WIDTH | NeedsLayout::CONTENT);
  }
  if (child->needs_layout().has(NeedsLayout::HEIGHT) &&
      base::holds_alternative<LayoutWrapContent>(lp->height_layout_spec())) {
    mark_needs_layout(NeedsLayout::HEIGHT | NeedsLayout::CONTENT);
  }
}

}  // namespace cursedui::view
