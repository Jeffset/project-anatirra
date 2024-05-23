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

#include "cursedui/config.hpp"

#include <chrono>

namespace cursedui::animation {
using namespace std::chrono_literals;

class CURSEDUI_PUBLIC Animation : public base::WeakReferenced {
 public:
  using clock_t = std::chrono::steady_clock;
  using duration_t = clock_t::duration;
  using time_point_t = clock_t::time_point;

  static constexpr duration_t interval_v = 50ms;

 public:
  bool is_finished() const noexcept { return finished_; }

 private:
  friend class AnimationHost;

  void set_attached(bool attached) noexcept;
  bool is_attached() const noexcept { return attached_; }
  bool attached_;

 protected:
  Animation() noexcept;
  ~Animation() noexcept override;

  virtual void on_frame() = 0;

  bool finished_;
};

}  // namespace cursedui::animation