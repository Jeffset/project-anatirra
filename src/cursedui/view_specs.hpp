// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_SPECS
#define ANATIRRA_CURSEDUI_VIEW_SPECS

#include "base/util.hpp"
#include "cursedui/dim.hpp"

#include <type_traits>
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

enum NeedsLayoutMark : uint8_t {
  // clang-format off
  NEEDS_LAYOUT_NOT     = 0b000,
  NEEDS_LAYOUT_CONTENT = 0b001,
  NEEDS_LAYOUT_WIDTH   = 0b010,
  NEEDS_LAYOUT_HEIGHT  = 0b100,
  NEEDS_LAYOUT_SIZE    = 0b110,
  // clang-format on
};

using NeedsLayoutMarkBin = std::underlying_type_t<NeedsLayoutMark>;

MeasureSpec make_measure_spec(const LayoutSpec& layout,
                              MeasureSpec parent_measure) noexcept;

MeasureSpec shrink_measure_spec(MeasureSpec spec, gfx::dim_t dim) noexcept;

NeedsLayoutMarkBin make_layout_propagation_mask(const LayoutSpec& layout,
                                                MeasureSpec parent_measure,
                                                NeedsLayoutMarkBin mark) noexcept;

inline auto measure_exactly_or(gfx::dim_t value) {
  return base::overloaded{[](MeasureExactly exaclty) { return exaclty.dim; },
                          [value](auto) { return value; }};
}

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEW_SPECS
