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

#include "cursedui/view_specs.hpp"

#include "base/util.hpp"

namespace cursedui::view {

MeasureSpec make_measure_spec(LayoutSpec layout, MeasureSpec parent_measure) noexcept {
  constexpr auto maker = base::overloaded{
      [](LayoutExactly exaclty, auto) -> MeasureSpec {
        return MeasureExactly{{exaclty.dim}};
      },
      [](LayoutWrapContent, MeasureUnlimited unlimited) -> MeasureSpec {
        return unlimited;
      },
      [](LayoutWrapContent, MeasureSpecified specified) -> MeasureSpec {
        return MeasureAtMost{{specified.dim}};
      },
      [](LayoutMatchParent, auto parent) -> MeasureSpec { return parent; },
  };
  return std::visit(maker, layout, parent_measure);
}

gfx::dim_t fix_measure(gfx::dim_t dim, MeasureSpec spec) noexcept {
  return std::visit(
      base::overloaded{
          [](MeasureExactly ex) { return ex.dim; },
          [dim](MeasureAtMost at_most) { return std::min(dim, at_most.dim); },
          [dim](MeasureUnlimited) { return dim; },
      },
      spec);
}

MeasureSpec shrink_measure_spec(MeasureSpec spec, gfx::dim_t dim) noexcept {
  return std::visit(
      base::overloaded{
          [dim](MeasureExactly exactly) -> MeasureSpec {
            return MeasureExactly{{exactly.dim - dim}};
          },
          [dim](MeasureAtMost at_most) -> MeasureSpec {
            return MeasureAtMost{{at_most.dim - dim}};
          },
          [](MeasureUnlimited unlimited) -> MeasureSpec { return unlimited; },
      },
      spec);
}

base::EnumFlags<NeedsLayout> make_layout_propagation_mask(
    const LayoutSpec& layout,
    MeasureSpec parent_measure,
    base::EnumFlags<NeedsLayout> mark) noexcept {
  return std::visit(
      base::overloaded{
          // layout exactly never needs propagating size change.
          [](LayoutExactly) -> base::EnumFlags<NeedsLayout> { return NeedsLayout::NOT; },
          // wrap_content always propagates size change.
          [mark](LayoutWrapContent) -> base::EnumFlags<NeedsLayout> { return mark; },
          // match_parent is more complex case
          [&parent_measure, mark](LayoutMatchParent) -> base::EnumFlags<NeedsLayout> {
            return std::visit(
                base::overloaded{
                    // if parent is laid out exactly, do not propagate layout.
                    [](MeasureExactly) -> base::EnumFlags<NeedsLayout> {
                      return NeedsLayout::NOT;
                    },
                    // if for every other spec size may influence, so do propagate.
                    [mark](auto) -> base::EnumFlags<NeedsLayout> { return mark; },
                },
                parent_measure);
          },
      },
      layout);
}

bool operator==(const MeasureSpec& lhs, const MeasureSpec& rhs) noexcept {
  constexpr auto cmp = base::overloaded{
      [](const MeasureExactly& l, const MeasureExactly& r) { return l.dim == r.dim; },
      [](const MeasureAtMost& l, const MeasureAtMost& r) { return l.dim == r.dim; },
      [](MeasureUnlimited, MeasureUnlimited) { return true; },
      [](auto, auto) { return false; },
  };
  return std::visit(cmp, lhs, rhs);
}

bool operator==(const LayoutSpec& lhs, const LayoutSpec& rhs) noexcept {
  constexpr auto cmp = base::overloaded{
      [](LayoutMatchParent, LayoutMatchParent) { return true; },
      [](LayoutWrapContent, LayoutWrapContent) { return true; },
      [](LayoutExactly l, LayoutExactly r) { return l.dim == r.dim; },
      [](auto, auto) { return false; },
  };
  return std::visit(cmp, lhs, rhs);
}

}  // namespace cursedui::view
