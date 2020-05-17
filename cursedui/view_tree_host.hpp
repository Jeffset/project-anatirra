// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_VIEW_TREE_HOST
#define ANATIRRA_CURSEDUI_VIEW_TREE_HOST

#include "avada/avada.hpp"
#include "base/macro.hpp"
#include "cursedui/view.hpp"

#include <memory>

namespace cursedui::paint {
class Canvas;
class Region;
}  // namespace cursedui::paint

namespace cursedui {

class ViewTreeHost {
 public:
  ViewTreeHost(base::ref_ptr<view::View> root);

  void set_focused_view(base::ref_ptr<view::View> focused_view) noexcept;
  GETTER base::ref_ptr<view::View> focused_view() noexcept;

  void run();

 private:
  void layout_tree(paint::Region& repaint_region);
  bool paint_tree(paint::Region& paint_region, paint::Canvas& canvas);

 private:
  avada::Context avada_;

  const base::ref_ptr<view::View> root_;
  base::weak_ref<view::View> focused_view_;

  gfx::Size root_size_;
  bool need_root_resize_;

  DISABLE_COPY_AND_ASSIGN(ViewTreeHost);
};

}  // namespace cursedui

#endif  // ANATIRRA_CURSEDUI_VIEW_TREE_HOST
