//
// Created by jeffset on 12/15/19.
//

#ifndef CURSEDUI_VIEW_SPECS_HPP
#define CURSEDUI_VIEW_SPECS_HPP

#include "cursedui/dim.hpp"

#include <variant>

namespace cursedui::view {

struct MeasureSpecified {
  gfx::dim_t dim;
};
struct MeasureExactly : public MeasureSpecified {};
struct MeasureAtMost : public MeasureSpecified {};
struct MeasureUnlimited {};

using MeasureSpec = std::variant<MeasureExactly, MeasureAtMost, MeasureUnlimited>;

struct LayoutMatchParent {};
struct LayoutWrapContent {};
struct LayoutExactly {
  gfx::dim_t dim;
};

using LayoutSpec = std::variant<LayoutMatchParent, LayoutWrapContent, LayoutExactly>;

MeasureSpec make_measure_spec(const LayoutSpec& layout,
                              const MeasureSpec& parent_measure) noexcept;

MeasureSpec shrink_measure_spec(const MeasureSpec& spec, gfx::dim_t dim) noexcept;

}  // namespace cursedui::view

#endif  // CURSEDUI_VIEW_SPECS_HPP
