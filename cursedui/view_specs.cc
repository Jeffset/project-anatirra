// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/view_specs.hpp"

#include "base/util.hpp"

namespace cursedui::view {

MeasureSpec make_measure_spec(const LayoutSpec& layout,
                              MeasureSpec parent_measure) noexcept {
  return std::visit(
      base::overloaded{
          [](LayoutExactly exactly) -> MeasureSpec {
            return MeasureExactly{{exactly.dim}};
          },
          [&parent_measure](LayoutWrapContent) -> MeasureSpec {
            return std::visit(
                base::overloaded{
                    [](MeasureUnlimited) -> MeasureSpec { return MeasureUnlimited{}; },
                    [](MeasureSpecified spec) -> MeasureSpec {
                      return MeasureAtMost{{spec.dim}};
                    },
                },
                parent_measure);
          },
          [&parent_measure](LayoutMatchParent) -> MeasureSpec {
            return std::visit(
                base::overloaded{
                    [](MeasureUnlimited) -> MeasureSpec { return MeasureUnlimited{}; },
                    [](MeasureSpecified spec) -> MeasureSpec {
                      return MeasureExactly{{spec.dim}};
                    },
                },
                parent_measure);
          },
      },
      layout);
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

}  // namespace cursedui::view
