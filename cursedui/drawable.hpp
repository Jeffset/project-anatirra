#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include "base/ref_ptr.hpp"

namespace cursedui {

namespace rendering {
class Canvas;
}  // namespace rendering

class Drawable : base::RefCounted {
 protected:
  Drawable() noexcept;

 public:
  virtual void draw(rendering::Canvas&) = 0;
};

}  // namespace cursedui

#endif  // DRAWABLE_HPP
