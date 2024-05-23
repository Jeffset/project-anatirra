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

#define MARK_UNUSED(expr) (void)(expr)

#define DISABLE_COPY_AND_ASSIGN(Class) \
  Class(const Class&) = delete;        \
  Class& operator=(const Class&) = delete

#define DISABLE_MOVE_AND_ASSIGN(Class) \
  Class(Class&&) = delete;             \
  Class& operator=(Class&&) = delete

#define DISABLE_COPY_MOVE(Class)  \
  DISABLE_COPY_AND_ASSIGN(Class); \
  DISABLE_MOVE_AND_ASSIGN(Class)

#define NODISCARD [[nodiscard]]

#define GETTER NODISCARD

#define NONNULL [[gnu::__nonnull__]]

#define LIKELY(cond) __builtin_expect(bool(cond), 1)
#define UNLIKELY(cond) __builtin_expect(bool(cond), 0)