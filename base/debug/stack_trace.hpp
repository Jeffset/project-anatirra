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

#include "base/config.hpp"

#include <sstream>
#include <string>

namespace base::debug {

class BASE_PUBLIC StackTrace {
 public:
  StackTrace() noexcept;

  std::string to_string() const noexcept;
  std::string to_string(std::string_view preambula) const noexcept;

  constexpr static size_t MAX_TRACES = 32;

 private:
  std::ostringstream oss_;
};

}  // namespace base::debug

