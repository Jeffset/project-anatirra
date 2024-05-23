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

#include "cursedui/view_data.hpp"

#include "base/util.hpp"
#include "cursedui/view.hpp"

namespace cursedui::view {

ViewData::ViewData() noexcept : owner_(nullptr) {}

void ViewData::mark_needs_layout(NeedsLayout mark) noexcept {
  if (!owner_) {
    return;
  }
  owner_->mark_needs_layout(mark);
}

void ViewData::mark_needs_repaint() noexcept {
  if (!owner_) {
    return;
  }
  owner_->mark_needs_paint();
}

void ViewData::owned_by(View* view) noexcept {
  ASSERT(owner_ == nullptr);
  owner_ = view;
}

}  // namespace cursedui::view
