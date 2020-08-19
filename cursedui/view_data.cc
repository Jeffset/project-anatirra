// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/view_data.hpp"

#include "base/util.hpp"
#include "cursedui/view.hpp"

namespace cursedui::view {

ViewData::ViewData() : owner_(nullptr) {}

void ViewData::mark_needs_layout(NeedsLayout mark) {
  if (!owner_) {
    return;
  }
  owner_->mark_needs_layout(mark);
}

void ViewData::mark_needs_repaint() {
  if (!owner_) {
    return;
  }
  owner_->mark_needs_paint();
}

void ViewData::owned_by(View* view) {
  ASSERT(owner_ == nullptr);
  owner_ = view;
}

}  // namespace cursedui::view
