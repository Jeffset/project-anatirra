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

#include "cursedui/config.hpp"

#include "cursedui/dim.hpp"

#include <list>

namespace cursedui::paint {

class CURSEDUI_PUBLIC Region {
 public:
  Region() noexcept = default;
  explicit Region(const gfx::Rect& rect) noexcept;

  void add(const gfx::Rect& rect) noexcept;
  void add(const Region& region) noexcept;
  void add(Region&& region) noexcept;

  Region clip(const gfx::Rect& rect) const noexcept;
  Region clip(const Region& region) const noexcept;
  bool clip(const gfx::Point& point) const noexcept;

  bool empty() const noexcept;

  auto begin() const noexcept { return rects_.cbegin(); }
  auto end() const noexcept { return rects_.cend(); }

 private:
  // TODO: Maybe use quadro-trees?
  std::list<gfx::Rect> rects_;
};

}  // namespace cursedui::paint
