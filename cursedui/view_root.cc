//
// Created by jeffset on 12/15/19.
//

#include "cursedui/view_root.hpp"

#include "base/macro.hpp"
#include "cursedui/context.hpp"
#include "cursedui/rendering.hpp"

#include <utility>

namespace cursedui::view {

void ViewRoot::set_view_root(base::ref_ptr<View> root) noexcept {
  root_ = std::move(root);
}

ViewRoot::ViewRoot(Context* context) noexcept : context_(context) {}

void ViewRoot::render(render::Canvas& canvas, render::ColorPalette& palette) {
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
