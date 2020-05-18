// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_SPECS
#define ANATIRRA_CURSEDUI_VIEW_SPECS

#include "base/enum_flags.hpp"
#include "base/util.hpp"
#include "cursedui/dim.hpp"

#include "cursedui_config.hpp"

#include <type_traits>
#include <variant>

namespace cursedui::view {

struct CURSEDUI_PUBLIC MeasureSpecified {
  gfx::dim_t dim;
};
struct CURSEDUI_PUBLIC MeasureExactly : public MeasureSpecified {};
struct CURSEDUI_PUBLIC MeasureAtMost : public MeasureSpecified {};
struct CURSEDUI_PUBLIC MeasureUnlimited {};

using MeasureSpec = std::variant<MeasureExactly, MeasureAtMost, MeasureUnlimited>;

struct CURSEDUI_PUBLIC LayoutMatchParent {};
struct CURSEDUI_PUBLIC LayoutWrapContent {};
struct CURSEDUI_PUBLIC LayoutExactly {
  gfx::dim_t dim;
};

using LayoutSpec = std::variant<LayoutMatchParent, LayoutWrapContent, LayoutExactly>;

enum class CURSEDUI_PUBLIC NeedsLayout : uint8_t {
  // clang-format off
  NOT     = 0b000,
  CONTENT = 0b001,
  WIDTH   = 0b010,
  HEIGHT  = 0b100,
  SIZE    = 0b110,
  // clang-format on
};

CURSEDUI_PUBLIC MeasureSpec make_measure_spec(const LayoutSpec& layout,
                                              MeasureSpec parent_measure) noexcept;

CURSEDUI_PUBLIC MeasureSpec shrink_measure_spec(MeasureSpec spec,
                                                gfx::dim_t dim) noexcept;

CURSEDUI_PUBLIC base::EnumFlags<NeedsLayout> make_layout_propagation_mask(
    const LayoutSpec& layout,
    MeasureSpec parent_measure,
    base::EnumFlags<NeedsLayout> mark) noexcept;

inline auto measure_exactly_or(gfx::dim_t value) {
  return base::overloaded{[](MeasureExactly exaclty) { return exaclty.dim; },
                          [value](auto) { return value; }};
}

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEW_SPECS
