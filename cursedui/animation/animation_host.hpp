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

#pragma once

#include "base/macro.hpp"
#include "base/weak_ref.hpp"
#include "cursedui/animation/animation.hpp"

#include "cursedui/config.hpp"

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
