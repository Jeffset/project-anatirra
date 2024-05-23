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

#include "avada/color.hpp"
#include "base/enum_flags.hpp"
#include "cursedui/animation/animation.hpp"
#include "cursedui/view_data.hpp"

#include <optional>

namespace cursedui::animation {

void start_view_animation(view::View* view, Animation* animation);

template <class T>
class CURSEDUI_PUBLIC AnimationValue : public view::ViewData {
 public:
  enum Influence {
    // clang-format off
    REPAINT         = 0b001,
    RELAYOUT_WIDTH  = 0b010,
    RELAYOUT_HEIGHT = 0b100,
    // clang-format on
  };

  AnimationValue() noexcept : value_(), influence_(REPAINT) {}
  AnimationValue(T value, base::EnumFlags<Influence> influence) noexcept
      : value_(std::move(value)), influence_(influence) {}
  AnimationValue(T value) noexcept : value_(std::move(value)), influence_(REPAINT) {}

  operator const T&() const noexcept { return value_; }

  void set(T value) noexcept;

 private:
  T value_;
  base::EnumFlags<Influence> influence_;
};

template <class L>
class CURSEDUI_PUBLIC CustomAnimation final : public Animation {
 public:
  explicit CustomAnimation(L on_frame) noexcept : on_frame_(std::move(on_frame)) {}

 protected:
  void on_frame() final { on_frame_(); }

 private:
  const L on_frame_;
};

template <class T>
class CURSEDUI_PUBLIC ValueAnimation final : public Animation {
 public:
  ValueAnimation(AnimationValue<T>& value, T from, T to, duration_t duration) noexcept;
  ValueAnimation(AnimationValue<T>& value, T to, duration_t duration) noexcept;

 protected:
  void on_frame() final;

 private:
  AnimationValue<T>& value_;
  T from_;
  T to_;
  duration_t duration_;
  std::optional<time_point_t> start_time_;
};

using ColorAnimation = ValueAnimation<avada::render::ColorRGB>;
using DimAnimation = ValueAnimation<gfx::dim_t>;
using DoubleAnimation = ValueAnimation<double>;

}  // namespace cursedui::animation