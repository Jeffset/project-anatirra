// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_ANIMATION_ANIMATION_HOST
#define ANATIRRA_CURSEDUI_ANIMATION_ANIMATION_HOST

#include "base/macro.hpp"
#include "base/ref_ptr.hpp"
#include "cursedui/animation/animation.hpp"

#include "cursedui_config.hpp"

#include <chrono>
#include <list>

namespace cursedui {
class ViewTreeHost;
namespace view {
class View;
}
}  // namespace cursedui

namespace cursedui::animation {

class CURSEDUI_PUBLIC AnimationHost {
 public:
  AnimationHost() noexcept;

  DISABLE_COPY_MOVE(AnimationHost);

  void start(Animation* animation);

 private:
  friend class cursedui::ViewTreeHost;

  void tick();

  Animation::time_point_t last_tick_;
  std::list<base::weak_ref<Animation>> animations_;
};

}  // namespace cursedui::animation

#endif  // ANATIRRA_CURSEDUI_ANIMATION_ANIMATION_HOST
