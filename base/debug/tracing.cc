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

#include "base/debug/tracing.hpp"

#include "base/debug/debug.hpp"

namespace base::debug {

ScopedTrace::ScopedTrace(std::string_view name) noexcept
    : name_{name}, start_{std::chrono::high_resolution_clock::now()} {}

ScopedTrace::~ScopedTrace() noexcept {
  using namespace std::chrono;
  auto duration = high_resolution_clock::now() - start_;
  TRACE() << "trace (" << name_ << "): " << duration_cast<microseconds>(duration).count()
          << L" μs";
}

}  // namespace base::debug
