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
#include "base/run_loop.hpp"
#include "cursedui/animation/animations.hpp"
#include "cursedui/views/frame_layout.hpp"

#include "cursedui/config.hpp"

namespace cursedui::view {

class CURSEDUI_PUBLIC ScrollView : public FrameLayout {
 public:
  using LayoutParams = view::LayoutParams;

  enum class ScrollDirection {
    VERTICAL = 0b01,
    HORIZONTAL = 0b10,
  };

  ScrollView() noexcept;
  ~ScrollView() noexcept override;

  GETTER base::EnumFlags<ScrollDirection> direction() const noexcept {
    return direction_;
  }
  void set_direction(base::EnumFlags<ScrollDirection> direction) noexcept;

 protected:
  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;

  bool intercept_mouse_event(const avada::input::MouseEvent& event) override;
  void on_mouse_event(const avada::input::MouseEvent& event) override;
  void on_draw(paint::Canvas& canvas) override;

 private:
  void scroll_by(gfx::dim_t dx, gfx::dim_t dy) noexcept;

 private:
  base::ref_ptr<animation::Animation> scroll_fade_in_, scroll_fade_out_;
  base::ref_ptr<base::RunLoop::DelayedTask> hide_scroll_bar_;
  animation::AnimationValue<double> scroll_bar_opacity_;

  base::EnumFlags<ScrollDirection> direction_;
  gfx::Size scroll_offset_;
  gfx::Size max_scroll_offset_;
};

}  // namespace cursedui::view