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

#include "avada/write.hpp"

#include "base/exception.hpp"
#include "base/macro.hpp"

#include <unistd.h>

namespace avada::internal {

void write_stdout(const char* data, std::size_t count) {
  if (UNLIKELY(count == 0))
    return;

  long remaining_count = count;
  const char* remaining_data = data;

  while (remaining_count > 0) {
    auto written = ::write(STDOUT_FILENO, remaining_data, remaining_count);
    if (UNLIKELY(written < 0))
      throw base::system_exception("'write' call failed.");
    remaining_count -= written;
    remaining_data += written;
  }
}

void write_stdout(std::string_view data) {
  write_stdout(data.data(), data.size());
}

}  // namespace avada::internal
