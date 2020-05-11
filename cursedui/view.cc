// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

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
  measured_size_.reset();
  auto double_border_width = border() ? border_->border_width() * 2 : 0;
  if (double_border_width > 0) {
    on_measure(shrink_measure_spec(width_spec, double_border_width),
               shrink_measure_spec(height_spec, double_border_width));
  } else {
    on_measure(width_spec, height_spec);
  }

  ASSERT(measured_size_.has_value());

  if (double_border_width > 0) {
    measured_size_->width += double_border_width;
    measured_size_->height += double_border_width;
  }

  // Assert that measuring is done according to specs.
  // NOTE: Think: is this actually right idea to enforce it?
  std::visit(base::overloaded{
                 [this](const MeasureExactly& exactly) {
                   ASSERT(measured_size_->width == exactly.dim)
                       << "Measure exactly width spec is violated";
                 },
                 [this](const MeasureAtMost& at_most) {
                   ASSERT(measured_size_->width <= at_most.dim)
                       << "At most width spec is violated";
                 },
                 [](MeasureUnlimited) {},
             },
             width_spec);
  std::visit(base::overloaded{
                 [this](const MeasureExactly& exactly) {
                   ASSERT(measured_size_->height == exactly.dim)
                       << "Measure exactly height spec is violated";
                 },
                 [this](const MeasureAtMost& at_most) {
                   ASSERT(measured_size_->height <= at_most.dim)
                       << "At most height spec is violated";
                 },
                 [](MeasureUnlimited) {},
             },
             height_spec);
}

void View::layout(const gfx::Rect& area) {
  bounds_ = area;
  if (background_)
    background_->set_bounds(inner_bounds());
  if (border_)
    border_->set_bounds(outer_bounds());
  needs_layout_ = NEEDS_LAYOUT_NOT;
  on_layout();
}

void View::draw(paint::Canvas& canvas) {
  on_draw(canvas);
}

void View::dispatch_mouse_event(const avada::input::MouseEvent& event) {
  using namespace avada::input;
  LOG() << "Mouse event" << event.to_string();
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
  ASSERT(focusable());
  if (!view_tree_host_)
    return;
  if (focused())
    return;
  if (auto focused_view = view_tree_host_->focused_view()) {
    focused_view->unfocus();
  }
  view_tree_host_->set_focused_view(base::ref_ptr(this));
}

void View::unfocus() {
  if (focused()) {
    view_tree_host_->set_focused_view(nullptr);
  }
}

View::~View() = default;

View::View()
    : view_tree_host_(nullptr),
      background_(nullptr),
      border_(new BorderDrawable()),
      parent_(nullptr),
      needs_layout_(NEEDS_LAYOUT_SIZE),
      layout_propagation_mask(NEEDS_LAYOUT_SIZE) {}

base::nullable<LayoutParams> View::layout_params() const noexcept {
  return layout_params_.get();
}

void View::set_layout_params(std::unique_ptr<LayoutParams> layout_params) {
  layout_params_ = std::move(layout_params);
  mark_needs_layout(NEEDS_LAYOUT_SIZE);
}

void View::set_background(std::unique_ptr<Drawable> drawable) {
  background_ = std::move(drawable);
}

void View::set_background_color(avada::render::Color color) {
  background_ = std::make_unique<SolidColorDrawable>(color);
}

base::nullable<Drawable> View::background() {
  return background_.get();
}

void View::set_tree_host(base::nullable<ViewTreeHost> tree_host) {
  if (view_tree_host_ == tree_host)
    return;
  view_tree_host_ = tree_host.get_nullable();
  on_tree_host_set();
}

base::nullable<ViewTreeHost> View::tree_host() noexcept {
  return view_tree_host_;
}

base::nullable<ViewGroup> View::get_parent() {
  return parent_;
}

void View::set_parent(base::nullable<ViewGroup> parent) {
  parent_ = parent.get_nullable();
}

void View::mark_needs_layout(NeedsLayoutMarkBin mark) noexcept {
  needs_layout_ |= mark;
}

void View::visit_down(ViewTreeVisitor& visitor) {
  visitor.visit(this);
}

void View::visit_up(ViewTreeVisitor& visitor) {
  visitor.visit(this);
  if (parent_) {
    parent_->visit_up(visitor);
  }
}

void View::on_tree_host_set() {}

void View::set_measured_size(const gfx::Size& measured_size) {
  measured_size_ = measured_size;
}

void View::on_measure(MeasureSpec width_spec, MeasureSpec height_spec) {
  constexpr auto measurer = base::overloaded{
      [](const MeasureExactly& spec) { return spec.dim; },
      [](const auto&) { return 0; },
  };
  set_measured_size(
      {std::visit(measurer, width_spec), std::visit(measurer, height_spec)});
}

void View::on_layout() {}

void View::on_draw(paint::Canvas& canvas) {
  if (!outer_bounds().has_area())
    return;

  if (background_)
    background_->draw(canvas);

  if (border_)
    border_->draw(canvas);
}

gfx::Rect View::inner_bounds() const noexcept {
  // FIXME: a bit too heavy for a getter, isn't it?
  return border_ ? gfx::shrink(bounds_.value(), border_->border_width())
                 : bounds_.value();
}

gfx::Rect View::outer_bounds() const noexcept {
  return bounds_.value();
}

ViewTreeVisitor::~ViewTreeVisitor() = default;

}  // namespace cursedui::view
