// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEWS_LINEAR_LAYOUT
#define ANATIRRA_CURSEDUI_VIEWS_LINEAR_LAYOUT

#include "cursedui/view_group.hpp"

#include "cursedui_config.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace cursedui::view {

class CURSEDUI_PUBLIC LinearLayout : public ViewGroup {
 public:
  class LayoutParams : public view::LayoutParams {
   public:
    static const char* TAG;

    LayoutParams(LayoutSpec width,
                 LayoutSpec height,
                 base::EnumFlags<gfx::Gravity> gravity = gfx::Gravity::CENTER) noexcept;
    LayoutParams(LayoutSpec width,
                 LayoutSpec height,
                 float weight,
                 base::EnumFlags<gfx::Gravity> gravity = gfx::Gravity::CENTER) noexcept;

    void set_weight(float weight);
    GETTER std::optional<float> weight() const { return weight_; }

    void set_no_weight();

   private:
    GETTER std::string_view tag() const noexcept override;

   private:
    std::optional<float> weight_;
  };

  enum Orientation : unsigned char {
    HORIZONTAL = 0,
    VERTICAL = 1,
  };

  LinearLayout() noexcept;
  ~LinearLayout() noexcept override;

  GETTER Orientation orientation() const noexcept { return orientation_; }

  void set_orientation(Orientation orientation) noexcept;

  std::unique_ptr<view::LayoutParams> create_layout_params() const noexcept override;
  bool check_layout_params(view::LayoutParams* params) const noexcept override;

 protected:
  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;
  void propagate_needs_layout_mark(const View* child) override;

 private:
  Orientation orientation_;
  std::vector<View*> match_parent_children_tmp_;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEWS_LINEAR_LAYOUT
