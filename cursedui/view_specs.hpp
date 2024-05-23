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

#pragma once

#include "base/enum_flags.hpp"
#include "base/util.hpp"
#include "cursedui/dim.hpp"

#include "cursedui/config.hpp"

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

bool operator==(const MeasureSpec& lhs, const MeasureSpec& rhs) noexcept;

struct CURSEDUI_PUBLIC LayoutMatchParent {};
struct CURSEDUI_PUBLIC LayoutWrapContent {};
struct CURSEDUI_PUBLIC LayoutExactly {
  gfx::dim_t dim;
};

using LayoutSpec = std::variant<LayoutMatchParent, LayoutWrapContent, LayoutExactly>;

bool operator==(const LayoutSpec& lhs, const LayoutSpec& rhs) noexcept;

enum class CURSEDUI_PUBLIC NeedsLayout : uint8_t {
  // clang-format off

  // No layout required.
  NOT     = 0b000,

  // View wants to layout just its contents and that will not change its size.
  // NOTE: for the cases, when a View wants to relayout its size triggered by a content
  // change, it should mark itself both by CONTENT and WIDTH/HEIGHT flags.
  // Pure size flags should be used only if view doesn't need to relayout its contents by
  // themselfs.
  CONTENT = 0b001,

  // Conveys that width may be changed.
  WIDTH   = 0b010,

  // Conveys that height may be changed.
  HEIGHT  = 0b100,

  // Conveys, that either width or/and height may be changed.
  SIZE    = 0b110,

  // clang-format on
};

CURSEDUI_PUBLIC
MeasureSpec make_measure_spec(LayoutSpec layout, MeasureSpec parent_measure) noexcept;

CURSEDUI_PUBLIC
gfx::dim_t fix_measure(gfx::dim_t dim, MeasureSpec spec) noexcept;

CURSEDUI_PUBLIC
MeasureSpec shrink_measure_spec(MeasureSpec spec, gfx::dim_t dim) noexcept;

CURSEDUI_PUBLIC
base::EnumFlags<NeedsLayout> make_layout_propagation_mask(
    const LayoutSpec& layout,
    MeasureSpec parent_measure,
    base::EnumFlags<NeedsLayout> mark) noexcept;

inline auto measure_exactly_or(gfx::dim_t value) {
  return base::overloaded{[](MeasureExactly exaclty) { return exaclty.dim; },
                          [value](auto) { return value; }};
}

}  // namespace cursedui::view