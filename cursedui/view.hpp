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
#include "base/macro.hpp"
#include "base/nullable.hpp"
#include "base/weak_ref.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/view_specs.hpp"

#include "cursedui/config.hpp"

#include <memory>
#include <utility>

namespace avada::input {
class MouseEvent;
class KeyboardEvent;
}  // namespace avada::input

namespace cursedui {
class ViewTreeHost;
namespace paint {
class Canvas;
}  // namespace paint
namespace animation {
class Animation;
}
}  // namespace cursedui

namespace cursedui::view {

class ViewGroup;
class ViewTreeVisitor;
class LayoutParams;
class ViewData;

/**
 * Base class for cursed UI view system.
 */
class CURSEDUI_PUBLIC View : public base::WeakReferenced {
 public:
  View() noexcept;
  ~View() noexcept override;

  virtual void layout_as_root(const gfx::Rect& area);
  virtual void relayout();

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
  GETTER BorderDrawable& border() { return border_; }

  void set_tree_host(base::nullable<ViewTreeHost> tree_host);
  GETTER base::nullable<ViewTreeHost> tree_host() noexcept;

  GETTER base::nullable<ViewGroup> get_parent() const noexcept { return parent_; }
  void set_parent(base::nullable<ViewGroup> parent);

  void mark_needs_layout(base::EnumFlags<NeedsLayout> mark) noexcept;
  base::EnumFlags<NeedsLayout> needs_layout() const noexcept { return needs_layout_; }

  void mark_needs_paint() noexcept { needs_paint_ = true; }
  bool needs_paint() const noexcept { return needs_paint_; }

  virtual void visit_down(const ViewTreeVisitor& visitor);
  void visit_up(const ViewTreeVisitor& visitor);

  const std::string& debug_name() const noexcept { return debug_name_; }
  void set_debug_name(std::string debug_name) noexcept {
    debug_name_ = std::move(debug_name);
  }

 protected:
  virtual void on_tree_host_set() {}

  virtual void on_mouse_event(const avada::input::MouseEvent& event);

  virtual gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec);
  virtual void on_layout() {}
  virtual void on_draw(paint::Canvas& canvas);

  virtual void on_focus_changed(bool focused);

  void own_view_data(ViewData&);

 private:
  void dispatch_layout(bool changed);

 private:
  ViewTreeHost* view_tree_host_;

  std::optional<gfx::Size> measured_size_;
  std::optional<gfx::Rect> bounds_;

  BorderDrawable border_;
  std::unique_ptr<Drawable> background_;
  std::unique_ptr<LayoutParams> layout_params_;

  ViewGroup* parent_;

  base::EnumFlags<NeedsLayout> needs_layout_;
  bool needs_paint_;
  std::optional<MeasureSpec> last_width_spec_, last_height_spec_;

  std::string debug_name_;
};

enum class CURSEDUI_PUBLIC VisitResult : uint8_t { STOP_VISIT = 0, CONTINUE_VISIT = 1 };

class CURSEDUI_PUBLIC ViewTreeVisitor {
 protected:
  ~ViewTreeVisitor() = default;

 public:
  virtual VisitResult visit(View* view) const = 0;
};

}  // namespace cursedui::view

