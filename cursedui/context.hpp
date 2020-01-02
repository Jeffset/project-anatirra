//
// Created by jeffset on 12/9/19.
//

#ifndef CURSES_DEMO_CONTEXT_HPP
#define CURSES_DEMO_CONTEXT_HPP

#include <memory>

#include "cursedui/gfx.hpp"

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

  [[nodiscard]] gfx::Size screen_size() const;

 private:
  struct ContextImpl;
  std::unique_ptr<ContextImpl> impl_;
};

}  // namespace cursedui

#endif  // CURSES_DEMO_CONTEXT_HPP
