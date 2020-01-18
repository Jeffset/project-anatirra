//
// Created by jeffset on 12/15/19.
//

#include "cursedui/view_tree_host.hpp"

#include "base/macro.hpp"
#include "cursedui/context.hpp"
#include "cursedui/rendering.hpp"

#include <iostream>
#include <utility>

namespace cursedui::view {

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

ViewTreeHost::ViewTreeHost(Context* context) noexcept : context_(context) {}

void ViewTreeHost::render(render::Canvas& canvas, render::ColorPalette& palette) {
  const auto screen_size = context_->screen_size();
  view::MeasureSpec w_spec = MeasureExactly{{screen_size.width}};
  view::MeasureSpec h_spec = MeasureExactly{{screen_size.height}};
  root_->measure(w_spec, h_spec);
  auto size = root_->measured_size();
  gfx::Rect bounds = gfx::rect_from({0, 0}, size);
  root_->layout(bounds);
  root_->colorize(palette);
  canvas.start();
  MARK_UNUSED(root_->draw(canvas));
}

}  // namespace cursedui::view
