// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_SRC_CURSEDUI_CONTEXT
#define ANATIRRA_SRC_CURSEDUI_CONTEXT

#include "base/macro.hpp"
#include "cursedui/dim.hpp"

#include <memory>

namespace cursedui {

namespace render {
class Canvas;
class ColorPalette;
}  // namespace render

namespace view {
class ViewTreeHost;
}

class Context final {
 public:
  Context();
  ~Context();

  void run(view::ViewTreeHost& view_tree_host);

  GETTER gfx::Size screen_size() const;

  MAKE_FULLY_STATIONAR(Context);

 private:
  PIMPL(Context);
};

}  // namespace cursedui

#endif  // ANATIRRA_SRC_CURSEDUI_CONTEXT
