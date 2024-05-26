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

#include <utility>

namespace base {

template<class M, class K, class VP>
auto get_or_default(M& map, const K& key, VP&& default_provider) -> M::mapped_type& {
  if (auto it = map.find(key); it != map.end()) {
    return it->second;
  } else {
    auto [i, _] = map.insert(std::pair(key, std::forward<VP>(default_provider)()));
    return i->second;
  }
}

}  // namespace base