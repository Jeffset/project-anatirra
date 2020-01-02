//
// Created by jeffset on 12/8/19.
//

#ifndef CURSES_DEMO_VIEW_HPP
#define CURSES_DEMO_VIEW_HPP

#include "view_specs.hpp"
#include "rendering.hpp"
#include "gfx.hpp"
#include "ref_ptr.hpp"
#include "macro.hpp"

#include <utility>
#include <memory>
#include <optional>


namespace cursedui::render {
class Canvas;
}

namespace cursedui::view {

class LayoutParams;

/**
 * Base class for cursed UI view system.
 */
class View : public base::RefCounted {
 public:
  View();
  ~View() override;

  void measure(const MeasureSpec& width_spec,
               const MeasureSpec& height_spec);
  void layout(const gfx::Rect& area);
  void draw(render::Canvas& canvas);

  GETTER gfx::Size measured_size() const { return measured_size_.value(); }

  GETTER gfx::Size size() const { return bounds_->size(); }

  GETTER gfx::Point position() const { return bounds_->position(); }

  GETTER gfx::Rect inner_bounds() const;
  GETTER gfx::Rect outer_bounds() const;

  GETTER wchar_t background() const { return background_; }

  void set_background(wchar_t b);

  GETTER LayoutParams* layout_params() const {
    return layout_params_.get();
  }

  void set_layout_params(std::unique_ptr<LayoutParams> layout_params);

  void set_border_style(render::BorderStyle const* border_style);

  GETTER render::BorderStyle const* border_style() const {
    return border_style_;
  }

 protected:
  void set_measured_size(const gfx::Size& measured_size);

  virtual void on_measure(const MeasureSpec& width_spec,
                          const MeasureSpec& height_spec);
  virtual void on_layout();
  virtual void on_draw(render::Canvas& canvas);

 private:
  std::optional<gfx::Size> measured_size_;
  std::optional<gfx::Rect> bounds_;

 private:
  wchar_t background_;
  render::BorderStyle const* border_style_;

  std::unique_ptr<LayoutParams> layout_params_;

  struct ViewImpl;
  std::unique_ptr<ViewImpl> impl_;
};

}

#endif //CURSES_DEMO_VIEW_HPP
