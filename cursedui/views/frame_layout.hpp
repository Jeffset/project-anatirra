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

#include "cursedui/view_group.hpp"

#include "cursedui/config.hpp"

#include <vector>

namespace cursedui::view {

class CURSEDUI_PUBLIC FrameLayout : public ViewGroup {
 public:
  using LayoutParams = view::LayoutParams;

  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;
  void propagate_needs_layout_mark(const View* child) override;

 private:
  std::vector<View*> match_parent_children_tmp_;
};

}  // namespace cursedui::view