// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/view.hpp"

#include "base/debug/debug.hpp"
#include "base/util.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/rendering.hpp"
#include "cursedui/view_group.hpp"
#include "cursedui/view_tree_host.hpp"

#include <iostream>

namespace cursedui::view {

PIMPL_DEFINE(View){};

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
                   if (measured_size_->width != exactly.dim)
                     throw measure_spec_violated_exception();
                 },
                 [this](const MeasureAtMost& at_most) {
                   if (measured_size_->width > at_most.dim)
                     throw measure_spec_violated_exception();
                 },
                 [](MeasureUnlimited) {},
             },
             width_spec);
  std::visit(base::overloaded{
                 [this](const MeasureExactly& exactly) {
                   if (measured_size_->height != exactly.dim)
                     throw measure_spec_violated_exception();
                 },
                 [this](const MeasureAtMost& at_most) {
                   if (measured_size_->height > at_most.dim)
                     throw measure_spec_violated_exception();
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

void View::colorize(render::ColorPalette& palette) {
  if (background_)
    background_->colorize(palette);
  if (border_)
    border_->colorize(palette);
  on_colorize(palette);
}

render::BgColorState View::draw(render::Canvas& canvas) {
  try {
    return on_draw(canvas);
  } catch (render::render_exception& e) {
    std::cerr << e.what() << '\n';
    return {};
  }
}

void View::dispatch_mouse_event(const input::MouseEvent& event) {
  std::wcerr << L"mevent " << (int)event.event_code << L'\n';
  if (focusable() && event.is_mouse_down()) {
    focus();
  }
  on_mouse_event(event);
}

void View::dispatch_scroll_event(const input::ScrollEvent& event) {
  on_scroll_event(event);
}

void View::on_key_event(const input::KeyEvent&) {}

void View::on_mouse_event(const input::MouseEvent&) {}

void View::on_scroll_event(const input::ScrollEvent&) {}

bool View::focused() const noexcept {
  return view_tree_host_ && view_tree_host_->focused_view() == this;
}

void View::focus() {
  std::wcerr << "focus()\n";
  if (!focusable())
    throw view_exception();  // FIXME: throw proper exception type here
  if (!view_tree_host_)
    return;
  if (focused())
    return;
  std::wcerr << "focus2()\n";
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
      layout_propagation_mask(NEEDS_LAYOUT_SIZE),
      PIMPL_INIT(View) {}

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

void View::set_background_color(render::ColorDescr color) {
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

void View::on_colorize(render::ColorPalette&) {}

render::BgColorState View::on_draw(render::Canvas& canvas) {
  if (!outer_bounds().has_area()) {
    return {};
  }

  render::BgColorState bg;
  if (background_)
    bg = background_->draw(canvas);

  if (border_)
    border_->draw(canvas);

  return bg;
}

gfx::Rect View::inner_bounds() const noexcept {
  // FIXME: a bit too heavy for a getter, isn't it?
  return border_ ? gfx::shrink(bounds_.value(), border_->border_width())
                 : bounds_.value();
}

gfx::Rect View::outer_bounds() const noexcept {
  return bounds_.value();
}

const char* measure_spec_violated_exception::what() const noexcept {
  return "Measure spec has been violated";
}

ViewTreeVisitor::~ViewTreeVisitor() = default;

}  // namespace cursedui::view
