//
// Created by jeffset on 12/9/19.
//

#ifndef CURSEDUI_CONTEXT_HPP
#define CURSEDUI_CONTEXT_HPP

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

#endif  // CURSEDUI_CONTEXT_HPP
