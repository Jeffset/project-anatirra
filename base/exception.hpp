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

#include "base/debug/stack_trace.hpp"

#include "base/config.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace base {

class BASE_PUBLIC exception : public std::exception {
 public:
  template <class... Args>
  explicit exception(Args&&... args)
      : message_{
            static_cast<const std::ostringstream&>((std::ostringstream() << ... << args))
                .str()} {}

  const char* what() const noexcept override;

  const debug::StackTrace& stack_trace() const noexcept { return stack_trace_; }

 private:
  debug::StackTrace stack_trace_;

 protected:
  std::string message_;
};

class BASE_PUBLIC system_exception : public exception {
 public:
  explicit system_exception(std::string_view message) noexcept;
};

}  // namespace base

