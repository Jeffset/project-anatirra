// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/view_tree_host.hpp"

#include "base/macro.hpp"
#include "base/util.hpp"
#include "cursedui/context.hpp"
#include "cursedui/rendering.hpp"
#include "cursedui/view_group.hpp"

#include <forward_list>
#include <iostream>
#include <type_traits>
#include <utility>

namespace cursedui::view {

namespace {

class ScanLayoutVisitor : public ViewTreeVisitor {
 public:
  ScanLayoutVisitor(std::forward_list<View*>& roots) noexcept : roots_(roots) {}

  VisitResult visit(View* view) override {
    if (view->needs_layout()) {
      roots_.push_front(view);
      return STOP_VISIT;
    }
    return CONTINUE_VISIT;
  }

 private:
  std::forward_list<View*>& roots_;
};

class PropagateLayoutVisitor : public ViewTreeVisitor {
 public:
  VisitResult visit(View* view) override {
    if (auto* parent = view->get_parent().get_nullable()) {
      parent->propagate_needs_layout_mark(view);
      return parent->needs_layout() ? CONTINUE_VISIT : STOP_VISIT;
    }
    return STOP_VISIT;
  }
};

}  // namespace

ViewTreeHost::ViewTreeHost() noexcept = default;

void ViewTreeHost::set_view_root(base::ref_ptr<View> root) noexcept {
  if (root_) {
    root_->set_tree_host(nullptr);
  }
  root_ = std::move(root);
  if (root_) {
    root_->set_tree_host(this);
  }
}

void ViewTreeHost::set_focused_view(base::ref_ptr<View> focused_view) noexcept {
  if (focused_view_ == focused_view)
    return;
  focused_view_ = base::weak_ref(focused_view);
  std::wcerr << "ViewTreeHost: view focused\n";
}

base::ref_ptr<View> ViewTreeHost::focused_view() noexcept {
  return focused_view_.get_ref_ptr();
}

void ViewTreeHost::dispatch_key_event(const input::KeyEvent& event) {
  if (event.key_code == input::Key::TAB) {
    // TODO: focused view switching logic
    return;
  }

  if (auto* view = focused_view_.get()) {
    view->on_key_event(event);
  }
}

void ViewTreeHost::dispatch_mouse_event(const input::MouseEvent& event) {
  if (!root_)
    return;
  root_->dispatch_mouse_event(event);
}

void ViewTreeHost::dispatch_scroll_event(const input::ScrollEvent& event) {
  if (!root_)
    return;
  root_->dispatch_scroll_event(event);
}

void ViewTreeHost::layout_tree() {
  if (!root_)
    return;

  std::forward_list<View*> roots_needing_layout;
  ScanLayoutVisitor scan_visitor{roots_needing_layout};

  // Step I: obtain all sub-roots of the view hierarchy that explcitily need layout
  root_->visit_down(scan_visitor);

  if (roots_needing_layout.empty())
    return;

  // Step II: propagate layout need from every root up the tree.
  PropagateLayoutVisitor propagate_visitor;
  for (auto& view : roots_needing_layout) {
    view->visit_up(propagate_visitor);
  }

  // Step III: re-scan the tree to obtain the final set of roots needing layout.
  roots_needing_layout.clear();
  root_->visit_down(scan_visitor);

  // Step IV: Perform actual layout on every final detected root.
  for (auto* view : roots_needing_layout) {
    const auto size = view->size();
    view->measure(MeasureExactly{{size.width}}, MeasureExactly{{size.height}});
    view->layout(gfx::rect_from(view->position(), size));
  }
}

void ViewTreeHost::on_terminal_resize(const gfx::Size& screen_size) {
  view::MeasureSpec w_spec = MeasureExactly{{screen_size.width}};
  view::MeasureSpec h_spec = MeasureExactly{{screen_size.height}};
  root_->measure(w_spec, h_spec);
  auto size = root_->measured_size();
  gfx::Rect bounds = gfx::rect_from({0, 0}, size);
  root_->layout(bounds);
}

void ViewTreeHost::poll(render::Canvas& canvas, render::ColorPalette& palette) {
  layout_tree();

  root_->colorize(palette);
  canvas.start();
  MARK_UNUSED(root_->draw(canvas));
}

}  // namespace cursedui::view
