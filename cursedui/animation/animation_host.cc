/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
