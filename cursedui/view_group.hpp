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

#include "base/macro.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view.hpp"
#include "cursedui/view_data.hpp"

#include "cursedui/config.hpp"

#include <list>
#include <memory>
#include <string_view>

namespace cursedui::view {

class LayoutParams;

class CURSEDUI_PUBLIC ViewGroup : public View {
 public:
  using LayoutParams = LayoutParams;

  ViewGroup() noexcept;
  ~ViewGroup() noexcept override;

  void layout_as_root(const gfx::Rect& area) final;
  void relayout() final;
  bool focusable() const noexcept final { return false; }

  void add_child(base::ref_ptr<View> child) noexcept;
  void add_child(base::ref_ptr<View> child,
                 std::unique_ptr<LayoutParams> layout_params) noexcept;

  void remove_child(base::ref_ptr<View> child) noexcept;

  auto begin() noexcept { return children_.begin(); }
  auto end() noexcept { return children_.end(); }

  void dispatch_mouse_event(const avada::input::MouseEvent& event) final;

  void visit_down(const ViewTreeVisitor& visitor) final;

  virtual void propagate_needs_layout_mark(const View* child) = 0;

 protected:
  virtual bool intercept_mouse_event(const avada::input::MouseEvent& event);

  virtual std::unique_ptr<LayoutParams> create_layout_params() const noexcept;
  virtual bool check_layout_params(LayoutParams* params) const noexcept;

  void on_tree_host_set() override;

  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override = 0;
  void on_layout() override = 0;
  void on_draw(paint::Canvas& canvas) override;

 private:
  using children_container_t = std::list<base::ref_ptr<View>>;
  void remove_child_internal(children_container_t::iterator child) noexcept;

 private:
  children_container_t children_;
};

class CURSEDUI_PUBLIC LayoutParams : public ViewData {
 public:
  static const char* TAG;

  GETTER LayoutSpec width_layout_spec() const noexcept { return width_; }
  GETTER LayoutSpec height_layout_spec() const noexcept { return height_; }
  GETTER base::EnumFlags<gfx::Gravity> gravity() const noexcept { return gravity_; }

  void set_width_layout_spec(const LayoutSpec& spec) noexcept;
  void set_height_layout_spec(const LayoutSpec& spec) noexcept;
  void set_gravity(base::EnumFlags<gfx::Gravity> gravity) noexcept;

  GETTER virtual std::string_view tag() const noexcept { return TAG; }

  LayoutParams(LayoutSpec width,
               LayoutSpec height,
               base::EnumFlags<gfx::Gravity> gravity = gfx::Gravity::CENTER) noexcept;
  virtual ~LayoutParams() noexcept = default;

 private:
  LayoutSpec width_, height_;
  base::EnumFlags<gfx::Gravity> gravity_;
};

template <class V, class... Args>
void add_child(const base::ref_ptr<V>& view,
               const base::ref_ptr<View>& child,
               Args&&... args) {
  view->add_child(
      child, std::make_unique<typename V::LayoutParams>(std::forward<Args>(args)...));
}

template <class V>
auto add_child(const base::ref_ptr<V>& view,
               const base::ref_ptr<View>& child) -> V::LayoutParams* {
  view->add_child(child);
  return static_cast<V::LayoutParams*>(child->layout_params().get());
}

}  // namespace cursedui::view

