//
// Created by jeffset on 12/15/19.
//

#ifndef CURSEDUI_VIEW_ROOT_HPP
#define CURSEDUI_VIEW_ROOT_HPP

#include "base/macro.hpp"
#include "cursedui/input.hpp"
#include "cursedui/view.hpp"

#include <memory>

namespace cursedui {
class Context;
namespace render {
class Canvas;
}  // namespace render
}  // namespace cursedui

namespace cursedui::view {

class ViewTreeHost {
 public:
  explicit ViewTreeHost(Context* context) noexcept;

  void set_view_root(base::ref_ptr<View> root) noexcept;

  void set_focused_view(base::ref_ptr<View> focused_view) noexcept;
  GETTER base::ref_ptr<View> focused_view() noexcept;

  void dispatch_key_event(const input::KeyEvent& event);
  void dispatch_mouse_event(const input::MouseEvent& event);
  void dispatch_scroll_event(const input::ScrollEvent& event);

  void render(render::Canvas& canvas, render::ColorPalette& palette);

 private:
  Context* context_;
  base::ref_ptr<View> root_;

  base::weak_ref<View> focused_view_;

  DISABLE_COPY_AND_ASSIGN(ViewTreeHost);
};

}  // namespace cursedui::view

#endif  // CURSEDUI_VIEW_ROOT_HPP
