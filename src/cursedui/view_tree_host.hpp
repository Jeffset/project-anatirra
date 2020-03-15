// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_SRC_CURSEDUI_VIEW_TREE_HOST
#define ANATIRRA_SRC_CURSEDUI_VIEW_TREE_HOST

#include "base/macro.hpp"
#include "cursedui/input.hpp"
#include "cursedui/view.hpp"

#include <memory>

namespace cursedui::render {
class Canvas;
class ColorPalette;
}  // namespace cursedui::render

namespace cursedui::view {

class ViewTreeHost {
 public:
  ViewTreeHost() noexcept;

  void set_view_root(base::ref_ptr<View> root) noexcept;

  void set_focused_view(base::ref_ptr<View> focused_view) noexcept;
  GETTER base::ref_ptr<View> focused_view() noexcept;

  void dispatch_key_event(const input::KeyEvent& event);
  void dispatch_mouse_event(const input::MouseEvent& event);
  void dispatch_scroll_event(const input::ScrollEvent& event);

  void on_terminal_resize(const gfx::Size& screen_size);

  void poll(render::Canvas& canvas, render::ColorPalette& palette);

 private:
  void layout_tree();

 private:
  base::ref_ptr<View> root_;

  base::weak_ref<View> focused_view_;

  DISABLE_COPY_AND_ASSIGN(ViewTreeHost);
};

}  // namespace cursedui::view

#endif  // ANATIRRA_SRC_CURSEDUI_VIEW_TREE_HOST
