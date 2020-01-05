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
}

class Context final {
 public:
  class Delegate {
   public:
    virtual void render(render::Canvas& canvas) = 0;
  };

  Context();
  ~Context();

  void run(Delegate* delegate);

  GETTER gfx::Size screen_size() const;

  MAKE_FULLY_STATIONAR(Context);

 private:
  struct ContextImpl;
  std::unique_ptr<ContextImpl> impl_;
};

}  // namespace cursedui

#endif  // CURSEDUI_CONTEXT_HPP
