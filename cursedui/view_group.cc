// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/view_group.hpp"

#include "avada/input.hpp"
#include "base/debug/debug.hpp"
#include "base/util.hpp"
#include "cursedui/drawable.hpp"

#include <algorithm>

namespace cursedui::view {

ViewGroup::ViewGroup() noexcept {
  border()->set_style(BorderDrawable::Style::NO_BORDER);
}

void ViewGroup::layout_as_root(const gfx::Rect& area) {
  ASSERT(get_parent() == nullptr) << "Can't layout as root if attached to parent.";
  const auto new_size = area.size();
  // It's ok to measure exactly here, for we are the root.
  measure(MeasureExactly{{new_size.width}}, MeasureExactly{{new_size.height}});
  layout(area);
}

void ViewGroup::relayout() {
  // This is safe to call even if we are down in a hierarchy.
  const auto current_size = size();
  measure(MeasureExactly{{current_size.width}}, MeasureExactly{{current_size.height}},
          /* do not update layout masks */ false);
  layout(gfx::rect_from(position(), current_size));
}

ViewGroup::~ViewGroup() noexcept {
  while (!children_.empty()) {
    remove_child_internal(children_.begin());
  }
}

void ViewGroup::add_child(base::ref_ptr<View> child) noexcept {
  ASSERT(child->get_parent() == nullptr);
  ASSERT(std::find(children_.begin(), children_.end(), child) == children_.end());

  auto old_lp = child->layout_params();
  if (!old_lp || !check_layout_params(old_lp.get())) {
    child->set_layout_params(create_layout_params());
  }
  child->set_tree_host(tree_host());
  child->set_parent(this);
  children_.emplace_back(std::move(child));
  layout_propagation_masks_[child.get()] = NeedsLayout::SIZE;

  // Explicitly mark this as needing full size layout, for we must do the first layout
  // of the new view and we do not yet know layout need propagation mask.
  mark_needs_layout(NeedsLayout::SIZE);
}

void ViewGroup::remove_child(base::ref_ptr<View> child) noexcept {
  ASSERT(child->get_parent() == this);

  // We know the layout propagation mask here, so we can use it, assuming that removing
  // the child is equivalent to setting it's size to {0, 0} or something.
  child->mark_needs_layout(NeedsLayout::SIZE);
  propagate_needs_layout_mark(child.get());
  layout_propagation_masks_.erase(child.get());

  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    remove_child_internal(it);
  }
}

void ViewGroup::remove_child_internal(children_container_t::iterator child) noexcept {
  const auto& child_ptr = *child;
  child_ptr->set_tree_host(nullptr);
  child_ptr->set_parent(nullptr);
  children_.erase(child);
}

void ViewGroup::dispatch_mouse_event(const avada::input::MouseEvent& event) {
  if (intercept_mouse_event(event)) {
    on_mouse_event(event);
    return;
  }
  auto pos = gfx::Point{event.x, event.y};
  for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
    auto& child = *it;
    if (child->outer_bounds().contains(pos)) {
      child->dispatch_mouse_event(event);
      break;
    }
  }
}

bool ViewGroup::intercept_mouse_event(const avada::input::MouseEvent&) {
  return false;
}

std::unique_ptr<LayoutParams> ViewGroup::create_layout_params() const noexcept {
  return std::make_unique<LayoutParams>(LayoutWrapContent{}, LayoutWrapContent{});
}

bool ViewGroup::check_layout_params(LayoutParams* params) const noexcept {
  return params->tag() == LayoutParams::TAG;
}

void ViewGroup::visit_down(const ViewTreeVisitor& visitor) {
  if (visitor.visit(this) == VisitResult::STOP_VISIT)
    return;
  for (auto& child : children_) {
    child->visit_down(visitor);
  }
}

void ViewGroup::propagate_needs_layout_mark(View* child) {
  ASSERT(child->get_parent() == this);
  auto propagated_mark = child->needs_layout() & layout_propagation_masks_[child];
  mark_needs_layout(propagated_mark);
}

void ViewGroup::on_tree_host_set() {
  for (auto& child : children_) {
    child->set_tree_host(tree_host());
  }
}

void ViewGroup::dispatch_layout(bool) {
  on_layout();
}

void ViewGroup::on_draw(paint::Canvas& canvas) {
  View::on_draw(canvas);
  for (auto& child : children_) {
    child->draw(canvas);
  }
}

const char* LayoutParams::TAG = "LayoutParams";

LayoutParams::LayoutParams(const LayoutSpec& width, const LayoutSpec& height) noexcept
    : width_(width), height_(height), gravity_(gfx::Gravity::CENTER) {}

}  // namespace cursedui::view
