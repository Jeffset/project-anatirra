/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cursedui/view.hpp"

#include "avada/input.hpp"
#include "base/debug/debug.hpp"
#include "base/util.hpp"
#include "cursedui/canvas.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/view_group.hpp"
#include "cursedui/view_tree_host.hpp"

namespace cursedui::view {

void View::measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  if (measured_size_ && !needs_layout() && last_width_spec_ == width_spec &&
      last_height_spec_ == height_spec) {
    LOG() << "Measure optimized out for '" << debug_name() << "'";
    return;
  }

  const auto double_border_width = border_.border_width() * 2;

  auto size = double_border_width > 0
                  ? on_measure(shrink_measure_spec(width_spec, double_border_width),
                               shrink_measure_spec(height_spec, double_border_width))
                  : on_measure(width_spec, height_spec);

  if (double_border_width > 0) {
    size.width += double_border_width;
    size.height += double_border_width;
  }

  // Assert that measuring is done according to specs.
  // NOTE: Think: is this actually right idea to enforce it?
  std::visit(base::overloaded{
                 [this, &size](const MeasureExactly& exactly) {
                   ASSERT(size.width == exactly.dim)
                       << "Measure exactly width spec is violated: " << size.width
                       << " vs " << exactly.dim << " in '" << debug_name() << '\'';
                 },
                 [&size](const MeasureAtMost& at_most) {
                   ASSERT(size.width <= at_most.dim) << "At most width spec is violated";
                 },
                 [](MeasureUnlimited) {},
             },
             width_spec);
  std::visit(base::overloaded{
                 [this, &size](const MeasureExactly& exactly) {
                   ASSERT(size.height == exactly.dim)
                       << "Measure exactly height spec is violated: " << size.height
                       << " vs " << exactly.dim << " in '" << debug_name() << '\'';
                 },
                 [&size](const MeasureAtMost& at_most) {
                   ASSERT(size.height <= at_most.dim)
                       << "At most height spec is violated";
                 },
                 [](MeasureUnlimited) {},
             },
             height_spec);

  last_width_spec_ = width_spec;
  last_height_spec_ = height_spec;
  measured_size_ = size;
}

void View::layout(const gfx::Rect& area) {
  const auto old_inner_bounds = bounds_.has_value() ? inner_bounds() : gfx::Rect{};
  const auto old_bounds = bounds_.has_value() ? outer_bounds() : gfx::Rect{};

  bounds_ = area;
  const bool changed = old_inner_bounds != inner_bounds() || old_bounds != outer_bounds();
  dispatch_layout(changed);
  needs_layout_ = NeedsLayout::NOT;
}

void View::draw(paint::Canvas& canvas) {
  needs_paint_ = false;

  auto culled = canvas.push_clip(outer_bounds());
  if (culled) {
    LOG() << "view '" << debug_name() << "' culled";
    return;
  }

  on_draw(canvas);
}

void View::dispatch_mouse_event(const avada::input::MouseEvent& event) {
  using namespace avada::input;
  std::visit(base::overloaded{
                 [this](MouseEvent::ButtonEvent be) {
                   if (be.code == MouseEvent::Button::LEFT &&
                       be.state == MouseEvent::State::PRESSED && focusable()) {
                     focus();
                   }
                 },
                 [](auto) {},
             },
             event.data);
  on_mouse_event(event);
}

void View::on_key_event(const avada::input::KeyboardEvent&) {}

void View::on_mouse_event(const avada::input::MouseEvent&) {}

bool View::focused() const noexcept {
  return view_tree_host_ && view_tree_host_->focused_view() == this;
}

void View::focus() {
  if (!view_tree_host_ || !focusable())
    return;
  if (focused())
    return;

  if (auto focused_view = view_tree_host_->focused_view())
    focused_view->unfocus();

  view_tree_host_->set_focused_view(base::ref_ptr(this));

  on_focus_changed(true);
}

void View::unfocus() {
  ASSERT(focused());
  view_tree_host_->set_focused_view(nullptr);

  on_focus_changed(false);
}

View::~View() noexcept = default;

View::View() noexcept
    : view_tree_host_(nullptr),
      border_{},
      background_(nullptr),
      parent_(nullptr),
      needs_layout_(NeedsLayout::SIZE),
      needs_paint_(true) {
  border_.owned_by(this);
}

void View::relayout() {
  // No need to call measure, we are not ViewGroup here.
  dispatch_layout(false);
  // Clear layout flag as we are not using View::layout here.
  needs_layout_ = NeedsLayout::NOT;
}

void View::layout_as_root(const gfx::Rect& area) {
  // No need to call measure, we are not ViewGroup here.
  layout(area);
}

base::nullable<LayoutParams> View::layout_params() const noexcept {
  return layout_params_.get();
}

void View::set_layout_params(std::unique_ptr<LayoutParams> layout_params) {
  ASSERT(layout_params);
  layout_params_ = std::move(layout_params);
  own_view_data(*layout_params_);
  mark_needs_layout(NeedsLayout::SIZE);
}

void View::set_background(std::unique_ptr<Drawable> drawable) {
  background_ = std::move(drawable);
  if (background_) {
    own_view_data(*background_);
  }
  mark_needs_paint();
}

void View::set_background_color(avada::render::Color color) {
  set_background(std::make_unique<SolidColorDrawable>(color));
}

base::nullable<Drawable> View::background() {
  return background_.get();
}

void View::set_tree_host(base::nullable<ViewTreeHost> tree_host) {
  if (view_tree_host_ == tree_host)
    return;

  if (focused()) {
    unfocus();
  }

  view_tree_host_ = tree_host.get_nullable();
  on_tree_host_set();

  // Invalidate view as it's attached to a new view tree.
  mark_needs_layout(NeedsLayout::SIZE);
  mark_needs_paint();
}

base::nullable<ViewTreeHost> View::tree_host() noexcept {
  return view_tree_host_;
}

void View::set_parent(base::nullable<ViewGroup> parent) {
  parent_ = parent.get_nullable();

  // Reset layout mask for the new parent should set a new one once the first new layout
  // is done.
  measured_size_.reset();
  // Invalidate view as it's attached to the new parent.
  mark_needs_layout(NeedsLayout::SIZE);
  mark_needs_paint();
}

void View::mark_needs_layout(base::EnumFlags<NeedsLayout> mark) noexcept {
  needs_layout_.add(mark);
}

void View::visit_down(const ViewTreeVisitor& visitor) {
  visitor.visit(this);
}

void View::visit_up(const ViewTreeVisitor& visitor) {
  visitor.visit(this);
  if (parent_) {
    parent_->visit_up(visitor);
  }
}

gfx::Size View::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  constexpr auto measurer = base::overloaded{
      [](const MeasureExactly& spec) { return spec.dim; },
      [](const auto&) { return 0; },
  };
  return {std::visit(measurer, width_spec), std::visit(measurer, height_spec)};
}

void View::dispatch_layout(bool changed) {
  if (changed || needs_layout_.has(NeedsLayout::CONTENT)) {
    // If layout has changed or "inner" layout (CONTENT flag) is requested, then perform
    // actual layout.
    on_layout();

    // Mark view as needs paint automatically.
    mark_needs_paint();
  } else {
    LOG() << "view '" << debug_name() << "' layout skipped";
  }
}

void View::on_draw(paint::Canvas& canvas) {
  const auto bounds = outer_bounds();
  if (!bounds.has_area())
    return;

  if (background_)
    background_->draw(canvas, bounds);

  border_.draw(canvas, bounds);
}

void View::on_focus_changed(bool focused) {
  MARK_UNUSED(focused);
}

void View::own_view_data(ViewData& data) {
  data.owned_by(this);
}

gfx::Rect View::inner_bounds() const noexcept {
  // FIXME: a bit too heavy for a getter, isn't it?
  return gfx::shrink(bounds_.value(), border_.border_width());
}

gfx::Rect View::outer_bounds() const noexcept {
  return bounds_.value();
}

}  // namespace cursedui::view
