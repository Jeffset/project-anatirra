// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW
#define ANATIRRA_CURSEDUI_VIEW

#include "base/macro.hpp"
#include "base/nullable.hpp"
#include "base/ref_ptr.hpp"
#include "cursedui/color.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/input.hpp"
#include "cursedui/view_specs.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

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

class ViewTreeHost;

class LayoutParams;

class view_exception : public std::exception {};

class measure_spec_violated_exception : public std::exception {
 public:
  const char* what() const noexcept override;
};

class ViewTreeVisitor;
class ViewGroup;

/**
 * Base class for cursed UI view system.
 */
class View : public base::RefCounted, public base::WeakReferenced {
 public:
  View();
  ~View() override;

  void measure(MeasureSpec width_spec, MeasureSpec height_spec);
  void layout(const gfx::Rect& area);
  void colorize(render::ColorPalette& palette);
  NODISCARD render::BgColorState draw(render::Canvas& canvas);

  virtual void dispatch_mouse_event(const input::MouseEvent& event);
  virtual void dispatch_scroll_event(const input::ScrollEvent& event);
  virtual void on_key_event(const input::KeyEvent& event);

  virtual bool focusable() const noexcept { return false; }
  bool focused() const noexcept;
  void focus();
  void unfocus();

  GETTER gfx::Size measured_size() const noexcept { return measured_size_.value(); }
  GETTER gfx::Size size() const noexcept { return bounds_->size(); }
  GETTER gfx::Point position() const noexcept { return bounds_->position(); }

  GETTER gfx::Rect inner_bounds() const noexcept;
  GETTER gfx::Rect outer_bounds() const noexcept;

  GETTER base::nullable<LayoutParams> layout_params() const noexcept;
  void set_layout_params(std::unique_ptr<LayoutParams> layout_params);

  void set_background(std::unique_ptr<Drawable> drawable);
  void set_background_color(render::ColorDescr color);

  GETTER base::nullable<Drawable> background();
  GETTER BorderDrawable* border() { return border_.get(); }

  void set_tree_host(base::nullable<ViewTreeHost> tree_host);
  GETTER base::nullable<ViewTreeHost> tree_host() noexcept;

  GETTER base::nullable<ViewGroup> get_parent();
  void set_parent(base::nullable<ViewGroup> parent);

  void mark_needs_layout(NeedsLayoutMarkBin mark) noexcept;
  NeedsLayoutMarkBin needs_layout() const noexcept { return needs_layout_; }

  virtual void visit_down(ViewTreeVisitor& visitor);
  void visit_up(ViewTreeVisitor& visitor);

 protected:
  virtual void on_tree_host_set();

  void set_measured_size(const gfx::Size& measured_size);

  virtual void on_mouse_event(const input::MouseEvent& event);
  virtual void on_scroll_event(const input::ScrollEvent& event);

  virtual void on_measure(MeasureSpec width_spec, MeasureSpec height_spec);
  virtual void on_layout();
  virtual void on_colorize(render::ColorPalette& palette);
  NODISCARD virtual render::BgColorState on_draw(render::Canvas& canvas);

 private:
  ViewTreeHost* view_tree_host_;

  std::optional<gfx::Size> measured_size_;
  std::optional<gfx::Rect> bounds_;

  std::unique_ptr<Drawable> background_;
  std::unique_ptr<BorderDrawable> border_;  // TODO: consider making this stored by value
  std::unique_ptr<LayoutParams> layout_params_;

  ViewGroup* parent_;

  NeedsLayoutMarkBin needs_layout_;

 public:
  NeedsLayoutMarkBin layout_propagation_mask;

 private:
  PIMPL(View);
};

class ViewTreeVisitor {
 protected:
  ~ViewTreeVisitor();

 public:
  enum VisitResult : uint8_t { STOP_VISIT = 0, CONTINUE_VISIT = 1 };

  virtual VisitResult visit(View* view) = 0;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEW
