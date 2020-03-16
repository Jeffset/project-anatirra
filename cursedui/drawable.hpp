// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_DRAWABLE
#define ANATIRRA_CURSEDUI_DRAWABLE

#include "cursedui/color.hpp"
#include "cursedui/dim.hpp"

#include <variant>

namespace cursedui {

namespace render {
class Canvas;
class ColorPalette;
}  // namespace render

class Drawable {
 protected:
  Drawable() noexcept;

 public:
  virtual ~Drawable() noexcept;

  void set_bounds(const gfx::Rect& bounds);

  virtual void colorize(render::ColorPalette& palette);
  NODISCARD virtual render::BgColorState draw(render::Canvas&) = 0;

  GETTER base::ref_ptr<render::Color> background_color() const { return background_; }
  GETTER gfx::Rect bounds() const { return bounds_; }

 protected:
  base::ref_ptr<render::Color> background_;

 private:
  gfx::Rect bounds_;
};

class SolidColorDrawable : public Drawable {
 public:
  SolidColorDrawable() noexcept;
  explicit SolidColorDrawable(render::ColorDescr color_descr) noexcept;
  void set_color(render::ColorDescr color_descr) { color_descr_ = color_descr; }
  GETTER render::ColorDescr color() const { return color_descr_; }

 public:
  void colorize(render::ColorPalette& palette) override;
  NODISCARD render::BgColorState draw(render::Canvas& canvas) override;

 private:
  render::ColorDescr color_descr_;
};

class BorderDrawable : public Drawable {
 public:
  enum Style {
    NO_BORDER = 0,
    SINGLE = 1,
    DOUBLE = 2,
  };

  BorderDrawable() noexcept;

  void set_style(Style style);
  GETTER Style style() const { return style_; }

  void set_color(render::ColorDescr color_descr) { color_descr_ = color_descr; }
  GETTER render::ColorDescr color() const { return color_descr_; }

  void set_background_color(std::optional<render::ColorDescr> color_descr);
  GETTER std::optional<render::ColorDescr> background_color() const {
    return background_descr_;
  }

  GETTER gfx::dim_t border_width() const;

 public:
  void colorize(render::ColorPalette& palette) override;
  render::BgColorState draw(render::Canvas& canvas) override;

 private:
  render::ColorDescr color_descr_;
  std::optional<render::ColorDescr> background_descr_;
  Style style_;
  base::ref_ptr<render::Color> foreground_;
};

}  // namespace cursedui

#endif  // ANATIRRA_CURSEDUI_DRAWABLE
