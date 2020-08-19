// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_DATA
#define ANATIRRA_CURSEDUI_VIEW_DATA

#include "base/macro.hpp"
#include "cursedui/view_specs.hpp"

#include "cursedui_config.hpp"

#include <unordered_set>

namespace cursedui::view {

class View;

class CURSEDUI_PUBLIC ViewData {
  DISABLE_COPY_MOVE(ViewData);

 protected:
  ViewData();
  virtual ~ViewData() = default;

 protected:
  void mark_needs_layout(NeedsLayout mark);
  void mark_needs_repaint();

 private:
  friend class View;

  void owned_by(View* view);

 private:
  View* owner_;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_VIEW_DATA
