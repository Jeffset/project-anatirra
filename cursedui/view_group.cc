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

ViewGroup::~ViewGroup() noexcept = default;

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
}

void ViewGroup::remove_child(base::ref_ptr<View>& child) noexcept {
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it);
  }
  if (child->focused()) {
    child->unfocus();
  }
  child->set_tree_host(nullptr);
  child->set_parent(nullptr);
}

View* ViewGroup::get_child(int index) {
  ASSERT(index >= 0 && static_cast<size_t>(index) <= children_.size());
  return children_[index].get();
}

void ViewGroup::dispatch_mouse_event(const avada::input::MouseEvent& event) {
  if (intercept_mouse_event(event)) {
    on_mouse_event(event);
    return;
  }
  auto pos = gfx::Point{event.x, event.y};
  for (auto& child : children_) {
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

void ViewGroup::visit_down(ViewTreeVisitor& visitor) {
  if (!visitor.visit(this))
    return;
  for (auto& child : children_) {
    child->visit_down(visitor);
  }
}

void ViewGroup::propagate_needs_layout_mark(View* child) {
  auto propagated_mark = child->needs_layout() & ~child->layout_propagation_mask;
  mark_needs_layout(propagated_mark);
}

void ViewGroup::on_tree_host_set() {
  for (auto& child : children_) {
    child->set_tree_host(tree_host());
  }
}

int ViewGroup::child_count() const noexcept {
  return children_.size();
}

void ViewGroup::on_draw(paint::Canvas& canvas) {
  View::on_draw(canvas);
  for (auto& child : children_) {
    child->draw(canvas);
  }
}

LayoutSpec LayoutParams::width_layout_spec() const noexcept {
  return width_;
}

LayoutSpec LayoutParams::height_layout_spec() const noexcept {
  return height_;
}

void LayoutParams::set_width_layout_spec(const LayoutSpec& spec) noexcept {
  width_ = spec;
}

void LayoutParams::set_height_layout_spec(const LayoutSpec& spec) noexcept {
  height_ = spec;
}

std::string_view LayoutParams::tag() const noexcept {
  return TAG;
}

const char* LayoutParams::TAG = "LayoutParams";

LayoutParams::LayoutParams(const LayoutSpec& width, const LayoutSpec& height) noexcept
    : width_(width), height_(height) {}

LayoutParams::~LayoutParams() noexcept = default;

}  // namespace cursedui::view
