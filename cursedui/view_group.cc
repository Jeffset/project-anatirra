//
// Created by jeffset on 12/15/19.
//

#include "cursedui/view_group.hpp"

#include "cursedui/drawable.hpp"

#include <algorithm>
#include <cassert>

namespace cursedui::view {

ViewGroup::ViewGroup() noexcept {
  border()->set_style(BorderDrawable::NO_BORDER);
}

ViewGroup::~ViewGroup() noexcept = default;

void ViewGroup::add_child(base::ref_ptr<View> child) {
  if (std::find(children_.begin(), children_.end(), child) != children_.end()) {
    throw view_already_present();
  }
  auto old_lp = child->layout_params();
  if (!old_lp || !check_layout_params(old_lp.get())) {
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

void ViewGroup::on_colorize(render::ColorPalette& palette) {
  View::on_colorize(palette);

  for (auto& child : children_) {
    child->colorize(palette);
  }
}

int ViewGroup::child_count() const noexcept {
  return children_.size();
}

render::BgColorState ViewGroup::on_draw(render::Canvas& canvas) {
  auto bg = View::on_draw(canvas);

  for (auto& child : children_) {
    MARK_UNUSED(child->draw(canvas));
  }
  return render::BgColorState{};
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

const char* view_already_present::what() const noexcept {
  return "View is already present in ViewGroup";
}

}  // namespace cursedui::view
