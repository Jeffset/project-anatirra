//
// Created by jeffset on 12/15/19.
//

#ifndef CURSES_DEMO_VIEW_SPECS_HPP
#define CURSES_DEMO_VIEW_SPECS_HPP

#include "gfx.hpp"

#include <variant>


namespace cursedui::view {

struct MeasureSpecified { gfx::dim_t dim; };
struct MeasureExactly : public MeasureSpecified {};
struct MeasureAtMost :  public MeasureSpecified  {};
struct MeasureUnlimited {};

using MeasureSpec = std::variant<MeasureExactly, MeasureAtMost, MeasureUnlimited>;


struct LayoutMatchParent {};
struct LayoutWrapContent {};
struct LayoutExactly { gfx::dim_t dim; };

using LayoutSpec = std::variant<LayoutMatchParent, LayoutWrapContent, LayoutExactly>;

MeasureSpec make_measure_spec(const LayoutSpec& layout,
                              const MeasureSpec& parent_measure);

MeasureSpec shrink_measure_spec(const MeasureSpec& spec, gfx::dim_t dim);

}

#endif //CURSES_DEMO_VIEW_SPECS_HPP
