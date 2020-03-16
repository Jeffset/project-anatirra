// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_SRC_CURSEDUI_VIEWS_FRAME_LAYOUT
#define ANATIRRA_SRC_CURSEDUI_VIEWS_FRAME_LAYOUT

#include "cursedui/view_group.hpp"

namespace cursedui::view {

class FrameLayout : public ViewGroup {
 public:
  void on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_SRC_CURSEDUI_VIEWS_FRAME_LAYOUT
