// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_TREE_HOST
#define ANATIRRA_CURSEDUI_VIEW_TREE_HOST

#include "avada/avada.hpp"
#include "base/macro.hpp"
#include "cursedui/view.hpp"

#include <memory>

namespace paint {
class Canvas;
}

namespace cursedui {

class ViewTreeHost {
 public:
  ViewTreeHost(base::ref_ptr<view::View> root);

  void set_focused_view(base::ref_ptr<view::View> focused_view) noexcept;
  GETTER base::ref_ptr<view::View> focused_view() noexcept;

  void run();

 private:
  void layout_tree();
  void paint_tree(paint::Canvas& canvas);

 private:
  void handle_root_size(const gfx::Size& size);

 private:
  avada::Context avada_;

  const base::ref_ptr<view::View> root_;
  base::weak_ref<view::View> focused_view_;

  DISABLE_COPY_AND_ASSIGN(ViewTreeHost);
};

}  // namespace cursedui

#endif  // ANATIRRA_CURSEDUI_VIEW_TREE_HOST
