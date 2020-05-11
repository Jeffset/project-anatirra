// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_GROUP
#define ANATIRRA_CURSEDUI_VIEW_GROUP

#include "avada/color.hpp"
#include "base/macro.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace cursedui::view {

class LayoutParams;

class ViewGroup : public View {
 public:
  ViewGroup() noexcept;
  ~ViewGroup() noexcept override;

  void add_child(base::ref_ptr<View> child) noexcept;
  void remove_child(base::ref_ptr<View>& child) noexcept;

  GETTER int child_count() const noexcept;
  GETTER View* get_child(int index);

  void dispatch_mouse_event(const avada::input::MouseEvent& event) override;

  virtual bool intercept_mouse_event(const avada::input::MouseEvent& event);

  virtual std::unique_ptr<LayoutParams> create_layout_params() const noexcept;
  virtual bool check_layout_params(LayoutParams* params) const noexcept;

  void visit_down(ViewTreeVisitor& visitor) override;

  void propagate_needs_layout_mark(View* child);

 protected:
  void on_tree_host_set() override;

  void on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override = 0;
  void on_layout() override = 0;
  void on_draw(paint::Canvas& canvas) override;

 private:
  std::vector<base::ref_ptr<View>> children_;
};

class LayoutParams {
 public:
  static const char* TAG;

  GETTER LayoutSpec width_layout_spec() const noexcept { return width_; }
  GETTER LayoutSpec height_layout_spec() const noexcept { return height_; }
  GETTER base::EnumFlags<gfx::Gravity> gravity() const noexcept { return gravity_; }

  void set_width_layout_spec(const LayoutSpec& spec) noexcept { width_ = spec; }
  void set_height_layout_spec(const LayoutSpec& spec) noexcept { height_ = spec; }
  void set_gravity(base::EnumFlags<gfx::Gravity> gravity) noexcept { gravity_ = gravity; }

  GETTER virtual std::string_view tag() const noexcept { return TAG; }

  LayoutParams(const LayoutSpec& width, const LayoutSpec& height) noexcept;
  virtual ~LayoutParams() noexcept;

 private:
  LayoutSpec width_;
  LayoutSpec height_;
  base::EnumFlags<gfx::Gravity> gravity_;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEW_GROUP
