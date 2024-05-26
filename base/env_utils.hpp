/* Copyright 2024 Fedor Ihnatkevich
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

#include <cstdlib>
#include <optional>
#include <string>

namespace base {

inline std::optional<std::string> get_env(const char* env_var) noexcept {
  if (auto* value = std::getenv(env_var)) {
    return std::string{value};
  } else { 
    return std::nullopt;
  }
}

inline std::optional<std::string> get_env(const std::string& env_var) noexcept {
  return get_env(env_var.data());
}

}