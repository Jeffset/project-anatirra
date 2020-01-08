//
// Created by jeffset on 12/8/19.
//

#ifndef CURSEDUI_VIEW_HPP
#define CURSEDUI_VIEW_HPP

#include "base/macro.hpp"
#include "base/nullable.hpp"
#include "base/ref_ptr.hpp"
#include "cursedui/color.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view_specs.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>

namespace cursedui {
class Drawable;
class BorderDrawable;
namespace render {
class Canvas;
class ColorPalette;
struct BorderStyle;
}  // namespace render
}  // namespace cursedui

namespace cursedui::view {

class LayoutParams;

class view_exception : public std::exception {};

class measure_spec_violated_exception : public std::exception {
 public:
  const char* what() const noexcept override;
};

/**
 * Base class for cursed UI view system.
 */
class View : public base::RefCounted, public base::WeakReferenced {
 public:
  View();
  ~View() override;

  void measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec);
  void layout(const gfx::Rect& area);
  void colorize(render::ColorPalette& palette);
  NODISCARD render::BgColorState draw(render::Canvas& canvas);

  GETTER gfx::Size measured_size() const noexcept { return measured_size_.value(); }
  GETTER gfx::Size size() const noexcept { return bounds_->size(); }
  GETTER gfx::Point position() const noexcept { return bounds_->position(); }

  GETTER gfx::Rect inner_bounds() const noexcept;
  GETTER gfx::Rect outer_bounds() const noexcept;

  GETTER base::nullable_ptr<LayoutParams> layout_params() const noexcept;
  void set_layout_params(std::unique_ptr<LayoutParams> layout_params);

  void set_background(std::unique_ptr<Drawable> drawable);
  void set_background_color(render::ColorDescr color);

  GETTER base::nullable_ptr<Drawable> background();
  GETTER BorderDrawable* border() { return border_.get(); }

 protected:
  void set_measured_size(const gfx::Size& measured_size);

  virtual void on_measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec);
  virtual void on_layout();
  virtual void on_colorize(render::ColorPalette& palette);
  NODISCARD virtual render::BgColorState on_draw(render::Canvas& canvas);

 private:
  std::optional<gfx::Size> measured_size_;
  std::optional<gfx::Rect> bounds_;

 private:
  std::unique_ptr<Drawable> background_;
  std::unique_ptr<BorderDrawable> border_;  // TODO: consider making this stored by value
  std::unique_ptr<LayoutParams> layout_params_;

  PIMPL(View);
};

}  // namespace cursedui::view

#endif  // CURSEDUI_VIEW_HPP
