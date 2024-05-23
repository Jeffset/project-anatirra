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
#include "cursedui/canvas.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view_data.hpp"

#include "cursedui/config.hpp"

namespace cursedui {

class CURSEDUI_PUBLIC Drawable : public view::ViewData {
 protected:
  Drawable() noexcept;

 public:
  virtual ~Drawable() noexcept;

  virtual void draw(paint::Canvas&, const gfx::Rect& bounds) noexcept = 0;
};

class CURSEDUI_PUBLIC SolidColorDrawable : public Drawable {
 public:
  SolidColorDrawable() noexcept;
  explicit SolidColorDrawable(avada::render::Color color) noexcept;

  void set_color(avada::render::Color color) noexcept;
  GETTER avada::render::Color color() const noexcept { return pen_.bg_color; }

 public:
  void draw(paint::Canvas& canvas, const gfx::Rect& bounds) noexcept override;

 private:
  paint::Pen pen_;
};

class CURSEDUI_PUBLIC BorderDrawable final : public Drawable {
 public:
  enum class Style {
    NO_BORDER = 0,
    SINGLE = 1,
    DOUBLE = 2,
  };

  BorderDrawable() noexcept;

  void set_style(Style style) noexcept;
  GETTER Style style() const noexcept { return style_; }

  void set_color(avada::render::Color color) noexcept;
  GETTER avada::render::Color color() const noexcept { return pen_.fg_color; }

  void set_background_color(avada::render::Color color) noexcept;
  GETTER avada::render::Color background_color() const noexcept { return pen_.bg_color; }

  GETTER gfx::dim_t border_width() const noexcept;

 public:
  void draw(paint::Canvas& canvas, const gfx::Rect& bounds) noexcept override;

 private:
  paint::Pen pen_;
  Style style_;
};

}  // namespace cursedui
