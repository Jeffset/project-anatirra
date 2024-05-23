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

#include "base/debug/debug.hpp"
#include "base/macro.hpp"

namespace base {

/**
 * @brief This simple wrapper is all about expressing the nullability of returned pointer.
 * Nothing more, nothing less.
 */
template <class T>
class nullable {
 public:
  /* implicit */ inline nullable(std::nullptr_t) noexcept : ptr_(nullptr) {}
  /* implicit */ inline nullable(T* ptr) : ptr_(ptr) {}

  inline operator bool() const noexcept { return ptr_ != nullptr; }

  NONNULL
  inline T* get() noexcept {
    ASSERT(ptr_);
    return ptr_;
  }

  inline T* get_nullable() noexcept { return ptr_; }

  inline bool operator==(nullable other) const noexcept { return ptr_ == other.ptr_; }

  inline bool operator!=(nullable other) const noexcept { return ptr_ != other.ptr_; }

  template <class D>
  friend bool operator==(nullable<D> a, D* b);
  template <class D>
  friend bool operator==(D* a, nullable<D> b);
  template <class D>
  friend bool operator!=(nullable<D> a, D* b);
  template <class D>
  friend bool operator!=(D* a, nullable<D> b);

 private:
  T* ptr_;
};

template <class T>
inline bool operator==(nullable<T> a, T* b) {
  return a.ptr_ == b;
}

template <class T>
inline bool operator==(T* a, nullable<T> b) {
  return a == b.ptr_;
}

template <class T>
inline bool operator!=(nullable<T> a, T* b) {
  return a.ptr_ != b;
}

template <class T>
inline bool operator!=(T* a, nullable<T> b) {
  return a != b.ptr_;
}

}  // namespace base

