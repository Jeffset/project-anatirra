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

#include "base/config.hpp"

#include <chrono>
#include <string_view>

namespace base::debug {

class BASE_PUBLIC ScopedTrace {
 public:
  ScopedTrace(std::string_view name) noexcept;
  ~ScopedTrace() noexcept;

  DISABLE_COPY_MOVE(ScopedTrace);

 private:
  std::string_view name_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}  // namespace base::debug

