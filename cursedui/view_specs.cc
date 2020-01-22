//
// Created by jeffset on 12/15/19.
//

#include "cursedui/view_specs.hpp"

#include "base/util.hpp"

namespace cursedui::view {

MeasureSpec make_measure_spec(const LayoutSpec& layout,
                              const MeasureSpec& parent_measure) noexcept {
  return std::visit(
      base::overloaded{
          [](LayoutExactly exactly) -> MeasureSpec {
            return MeasureExactly{{exactly.dim}};
          },
          [&parent_measure](LayoutWrapContent) -> MeasureSpec {
            return std::visit(base::overloaded{[](MeasureUnlimited) -> MeasureSpec {
                                                 return MeasureUnlimited{};
                                               },
                                               [](MeasureSpecified spec) -> MeasureSpec {
                                                 return MeasureAtMost{{spec.dim}};
                                               }},
                              parent_measure);
          },
          [&parent_measure](LayoutMatchParent) -> MeasureSpec {
            return std::visit(base::overloaded{[](MeasureUnlimited) -> MeasureSpec {
                                                 return MeasureUnlimited{};
                                               },
                                               [](MeasureSpecified spec) -> MeasureSpec {
                                                 return MeasureExactly{{spec.dim}};
                                               }},
                              parent_measure);
          }},
      layout);
}

MeasureSpec shrink_measure_spec(const MeasureSpec& spec, gfx::dim_t dim) noexcept {
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

NeedsLayoutMarkBin make_layout_propagation_mask(const LayoutSpec& layout,
                                                const MeasureSpec& parent_measure,
                                                NeedsLayoutMarkBin mark) noexcept {
  return std::visit(
      base::overloaded{
          // layout exactly never needs propagating size change.
          [](LayoutExactly) -> NeedsLayoutMarkBin { return 0; },
          // wrap_content always propagates size change.
          [mark](LayoutWrapContent) -> NeedsLayoutMarkBin { return mark; },
          // match_parent is more complex case
          [&parent_measure, mark](LayoutMatchParent) -> NeedsLayoutMarkBin {
            return std::visit(
                base::overloaded{
                    // if parent is laid out exactly, do not propagate layout.
                    [](MeasureExactly) -> NeedsLayoutMarkBin { return 0; },
                    // if for every other spec size may influence, so do propagate.
                    [mark](auto) -> NeedsLayoutMarkBin { return mark; }},
                parent_measure);
          }},
      layout);
}

}  // namespace cursedui::view
