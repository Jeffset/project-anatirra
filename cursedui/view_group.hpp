// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_GROUP
#define ANATIRRA_CURSEDUI_VIEW_GROUP

#include "avada/color.hpp"
#include "base/macro.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view.hpp"

#include "cursedui_config.hpp"

#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace cursedui::view {

class LayoutParams;

class CURSEDUI_PUBLIC ViewGroup : public View {
 public:
  ViewGroup() noexcept;
  ~ViewGroup() noexcept override;

  void layout_as_root(const gfx::Rect& area) final;
  void relayout() final;

  void add_child(base::ref_ptr<View> child) noexcept;
  void add_child(base::ref_ptr<View> child,
                 std::unique_ptr<LayoutParams> layout_params) noexcept;

  void remove_child(base::ref_ptr<View> child) noexcept;

  auto begin() noexcept { return children_.begin(); }
  auto end() noexcept { return children_.end(); }

  void dispatch_mouse_event(const avada::input::MouseEvent& event) final;

  virtual bool intercept_mouse_event(const avada::input::MouseEvent& event);

  virtual std::unique_ptr<LayoutParams> create_layout_params() const noexcept;
  virtual bool check_layout_params(LayoutParams* params) const noexcept;

  void visit_down(const ViewTreeVisitor& visitor) final;

  virtual void propagate_needs_layout_mark(const View* child) = 0;

 protected:
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

class CURSEDUI_PUBLIC LayoutParams {
 public:
  static const char* TAG;

  GETTER LayoutSpec width_layout_spec() const noexcept { return width_; }
  GETTER LayoutSpec height_layout_spec() const noexcept { return height_; }
  GETTER base::EnumFlags<gfx::Gravity> gravity() const noexcept { return gravity_; }

  void set_width_layout_spec(const LayoutSpec& spec) noexcept { width_ = spec; }
  void set_height_layout_spec(const LayoutSpec& spec) noexcept { height_ = spec; }
  void set_gravity(base::EnumFlags<gfx::Gravity> gravity) noexcept { gravity_ = gravity; }

  GETTER virtual std::string_view tag() const noexcept { return TAG; }

  LayoutParams(LayoutSpec width,
               LayoutSpec height,
               base::EnumFlags<gfx::Gravity> gravity = gfx::Gravity::CENTER) noexcept;
  virtual ~LayoutParams() noexcept = default;

 private:
  LayoutSpec width_;
  LayoutSpec height_;
  base::EnumFlags<gfx::Gravity> gravity_;
};

template <class V, class... Args>
void add_child(const base::ref_ptr<V>& view,
               const base::ref_ptr<View>& child,
               Args&&... args) {
  view->add_child(
      child, std::make_unique<typename V::LayoutParams>(std::forward<Args>(args)...));
}

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEW_GROUP
