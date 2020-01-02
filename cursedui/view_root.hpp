//
// Created by jeffset on 12/15/19.
//

#ifndef CURSES_DEMO_VIEW_ROOT_HPP
#define CURSES_DEMO_VIEW_ROOT_HPP

#include "context.hpp"
#include "view.hpp"
#include "macro.hpp"

#include <memory>


namespace cursedui::render {
class Canvas;
}

namespace cursedui::view {

class ViewRoot : public Context::Delegate {
 public:
  explicit ViewRoot(Context* context);

  void set_view_root(base::ref_ptr<View> root);

  void render(render::Canvas& canvas) override;

 private:
  Context* context_;
  base::ref_ptr<View> root_;

  DISABLE_COPY_AND_ASSIGN(ViewRoot);
};

}

#endif //CURSES_DEMO_VIEW_ROOT_HPP
