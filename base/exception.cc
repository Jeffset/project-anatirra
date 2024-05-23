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

#include "base/exception.hpp"

#include <format>

namespace base {

namespace debug {}  // namespace debug

const char* exception::what() const noexcept {
  return message_.c_str();
}

system_exception::system_exception(std::string_view message) noexcept {
  message_ =
      std::format("system error: {}\n{}",
                  std::system_error(errno, std::generic_category()).what(), message);
}

}  // namespace base
