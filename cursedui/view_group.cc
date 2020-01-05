//
// Created by jeffset on 12/15/19.
//

#include "cursedui/view_group.hpp"

#include <algorithm>
#include <cassert>

namespace cursedui::view {

ViewGroup::ViewGroup() noexcept {
  set_border_style(nullptr);
}

ViewGroup::~ViewGroup() noexcept = default;

void ViewGroup::add_child(base::ref_ptr<View> child) {
  auto it = std::find(children_.begin(), children_.end(), child);
  assert(it == children_.end());
  auto old_lp = child->layout_params();
  if (!old_lp.has_value() || !check_layout_params(old_lp.value())) {
    child->set_layout_params(create_layout_params());
  }
  children_.emplace_back(std::move(child));
}

void ViewGroup::remove_child(const base::ref_ptr<View>& child) {
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it);
  }
}

base::ref_ptr<View> ViewGroup::get_child(int index) {
  return children_.at(index);
}

int ViewGroup::child_count() const noexcept {
  return children_.size();
}

void ViewGroup::on_draw(render::Canvas& canvas) {
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

LayoutParams::LayoutParams(const LayoutSpec& width, const LayoutSpec& height) noexcept
    : width_(width), height_(height) {}

LayoutParams::~LayoutParams() noexcept = default;

}  // namespace cursedui::view
