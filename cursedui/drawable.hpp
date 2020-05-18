// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_DRAWABLE
#define ANATIRRA_CURSEDUI_DRAWABLE

#include "cursedui_config.hpp"

#include "avada/color.hpp"
#include "cursedui/canvas.hpp"
#include "cursedui/dim.hpp"

namespace cursedui {

class CURSEDUI_PUBLIC Drawable {
 protected:
  Drawable() noexcept;

 public:
  virtual ~Drawable() noexcept;

  void set_bounds(const gfx::Rect& bounds) noexcept { bounds_ = bounds; }
  GETTER gfx::Rect bounds() const noexcept { return bounds_; }

  virtual void draw(paint::Canvas&) noexcept = 0;

 private:
  gfx::Rect bounds_;
};

class CURSEDUI_PUBLIC SolidColorDrawable : public Drawable {
 public:
  SolidColorDrawable() noexcept;
  explicit SolidColorDrawable(avada::render::Color color) noexcept;

  void set_color(avada::render::Color color) noexcept { pen_.bg_color = color; }
  GETTER avada::render::Color color() const noexcept { return pen_.bg_color; }

 public:
  void draw(paint::Canvas& canvas) noexcept override;

 private:
  paint::Pen pen_;
};

class CURSEDUI_PUBLIC BorderDrawable : public Drawable {
 public:
  enum class Style {
    NO_BORDER = 0,
    SINGLE = 1,
    DOUBLE = 2,
  };

  BorderDrawable() noexcept;

  void set_style(Style style) noexcept;
  GETTER Style style() const noexcept { return style_; }

  void set_color(avada::render::Color color) noexcept { pen_.fg_color = color; }
  GETTER avada::render::Color color() const noexcept { return pen_.fg_color; }

  void set_background_color(avada::render::Color color) noexcept {
    pen_.bg_color = color;
  }
  GETTER avada::render::Color background_color() const noexcept { return pen_.bg_color; }

  GETTER gfx::dim_t border_width() const noexcept;

 public:
  void draw(paint::Canvas& canvas) noexcept override;

 private:
  paint::Pen pen_;
  Style style_;
};

}  // namespace cursedui

#endif  // ANATIRRA_CURSEDUI_DRAWABLE
