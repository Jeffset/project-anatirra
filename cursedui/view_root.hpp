//
// Created by jeffset on 12/15/19.
//

#ifndef CURSEDUI_VIEW_ROOT_HPP
#define CURSEDUI_VIEW_ROOT_HPP

#include "base/macro.hpp"
#include "cursedui/context.hpp"
#include "cursedui/view.hpp"

#include <memory>

namespace cursedui::render {
class Canvas;
}

namespace cursedui::view {

class ViewRoot : public Context::Delegate {
 public:
  explicit ViewRoot(Context* context) noexcept;

  void set_view_root(base::ref_ptr<View> root) noexcept;

  void render(render::Canvas& canvas, render::ColorPalette& palette) override;

 private:
  Context* context_;
  base::ref_ptr<View> root_;

  DISABLE_COPY_AND_ASSIGN(ViewRoot);
};

}  // namespace cursedui::view

#endif  // CURSEDUI_VIEW_ROOT_HPP
