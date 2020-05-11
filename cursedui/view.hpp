// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW
#define ANATIRRA_CURSEDUI_VIEW

#include "avada/color.hpp"
#include "base/exception.hpp"
#include "base/macro.hpp"
#include "base/nullable.hpp"
#include "base/weak_ref.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view_specs.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

namespace avada::input {
class MouseEvent;
class KeyboardEvent;
}  // namespace avada::input

namespace cursedui {
class Drawable;
class BorderDrawable;
class ViewTreeHost;
namespace paint {
class Canvas;
}  // namespace paint
}  // namespace cursedui

namespace cursedui::view {

class ViewGroup;
class ViewTreeVisitor;
class LayoutParams;

/**
 * Base class for cursed UI view system.
 */
class View : public base::RefCounted, public base::WeakReferenced {
 public:
  View();
  ~View() override;

  // TODO: mark these as noexcept.
  void measure(MeasureSpec width_spec, MeasureSpec height_spec);
  void layout(const gfx::Rect& area);
  void draw(paint::Canvas& canvas);

  virtual void dispatch_mouse_event(const avada::input::MouseEvent& event);
  virtual void on_key_event(const avada::input::KeyboardEvent& event);

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
  void set_background_color(avada::render::Color color);

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

  virtual void on_mouse_event(const avada::input::MouseEvent& event);

  virtual void on_measure(MeasureSpec width_spec, MeasureSpec height_spec);
  virtual void on_layout();
  virtual void on_draw(paint::Canvas& canvas);

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
