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

#include "cursedui/view.hpp"

#include "cursedui/config.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace cursedui::view {

class CURSEDUI_PUBLIC TextView : public View {
 public:
  TextView() noexcept;

  void set_text(std::string str) noexcept;

  void append_text(std::string_view str) noexcept;

  GETTER const std::string& get_text() const noexcept { return text_; }

  void set_gravity(base::EnumFlags<gfx::Gravity> gravity) noexcept;
  GETTER base::EnumFlags<gfx::Gravity> gravity() const noexcept { return gravity_; }

  void set_multiline(bool multiline) noexcept;
  bool multiline() const noexcept { return multiline_; }

  void set_text_color(avada::render::Color color) noexcept;
  GETTER avada::render::Color text_color() const noexcept { return text_color_; }

  bool focusable() const noexcept override { return false; }

 protected:
  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;

  void on_draw(paint::Canvas& canvas) override;

  void on_focus_changed(bool focused) override;

 private:
  gfx::Size measure_text(gfx::dim_t max_width, gfx::dim_t max_height) const noexcept;

 private:
  // view attributes:
  std::string text_;
  base::EnumFlags<gfx::Gravity> gravity_;
  bool multiline_;
  avada::render::Color text_color_;

  // layout scoped attributes:
  std::vector<std::string_view> lines_to_render_;
  gfx::Point text_pos_;
  bool ellipsize_;
};

}  // namespace cursedui::view