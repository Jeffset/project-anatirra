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

#include "avada/avada.hpp"
#include "avada/input.hpp"
#include "base/macro.hpp"
#include "cursedui/animation/animation_host.hpp"
#include "cursedui/view.hpp"

#include "cursedui/config.hpp"

#include <functional>

namespace cursedui::paint {
class Canvas;
class Region;
}  // namespace cursedui::paint

namespace cursedui {

class CURSEDUI_PUBLIC ViewTreeHost {
 public:
  // returns `true` if the key is handled and should not be passed down
  using keyboard_handler_t = std::function<bool(const avada::input::KeyboardEvent&)>;

  ViewTreeHost(base::ref_ptr<view::View> root);

  void set_focused_view(base::ref_ptr<view::View> focused_view) noexcept;
  GETTER base::ref_ptr<view::View> focused_view() const noexcept;

  GETTER animation::AnimationHost& animation_host() noexcept { return animation_host_; }

  void set_keyboard_handler(keyboard_handler_t handler) noexcept {
    keyboard_handler_ = handler;
  }

  void tick();

 private:
  void layout_tree(paint::Region& repaint_region);
  bool paint_tree(paint::Region& paint_region, paint::Canvas& canvas);

  void work_routine();
  void view_tree_routine();

 private:
  avada::Context avada_;

  base::weak_ref<view::View> focused_view_;
  animation::AnimationHost animation_host_;
  
  // Order is important
  const base::ref_ptr<view::View> root_;

  keyboard_handler_t keyboard_handler_;
  gfx::Size root_size_;
  bool need_root_resize_;
  DISABLE_COPY_AND_ASSIGN(ViewTreeHost);
};

}  // namespace cursedui