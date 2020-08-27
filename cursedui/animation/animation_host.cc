// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/animation/animation_host.hpp"

#include "base/debug/debug.hpp"
#include "base/util.hpp"

namespace cursedui::animation {

AnimationHost::AnimationHost() noexcept = default;

void AnimationHost::tick() {
  const auto now = Animation::clock_t::now();
  const auto delta = now - last_tick_;
  if (delta >= Animation::interval_v) {
    last_tick_ = now;
    for (auto& animation : animations_) {
      if (auto anim = animation.lock()) {
        anim->on_frame();
      }
    }
    animations_.remove_if([](auto animation) {
      auto anim = animation.lock();
      return !anim || anim->is_finished();
    });
  }
}

void AnimationHost::start(Animation* animation) {
  ASSERT(!animation->is_attached()) << "Can't start already running animation";

  if (animation->is_finished())
    return;

  animations_.emplace_back(animation);
}

}  // namespace cursedui::animation
