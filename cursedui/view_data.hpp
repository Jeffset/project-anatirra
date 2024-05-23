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
#include "cursedui/view_specs.hpp"

#include "cursedui/config.hpp"

namespace cursedui::view {

class View;

class CURSEDUI_PUBLIC ViewData {
  DISABLE_COPY_MOVE(ViewData);

 protected:
  ViewData() noexcept;
  ~ViewData() noexcept = default;

 protected:
  void mark_needs_layout(NeedsLayout mark) noexcept;
  void mark_needs_repaint() noexcept;

 private:
  friend class View;

  void owned_by(View* view) noexcept;

 private:
  View* owner_;
};

}  // namespace cursedui::view
