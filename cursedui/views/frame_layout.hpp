// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEWS_FRAME_LAYOUT
#define ANATIRRA_CURSEDUI_VIEWS_FRAME_LAYOUT

#include "cursedui/view_group.hpp"

#include "cursedui_config.hpp"

#include <vector>

namespace cursedui::view {

class CURSEDUI_PUBLIC FrameLayout : public ViewGroup {
 public:
  using LayoutParams = view::LayoutParams;

  gfx::Size on_measure(MeasureSpec width_spec,
                       MeasureSpec height_spec,
                       bool update_layout_masks) override;
  void on_layout() override;

 private:
  std::vector<View*> match_parent_children_tmp_;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEWS_FRAME_LAYOUT
