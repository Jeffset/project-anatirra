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

#include <atomic>
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>

namespace base::internal {

enum class ThreadSafe {
  NOT = 0,
  SAFE = 1,
};

template <bool is_const, ThreadSafe safe>
class ref_ptr_base;

template <ThreadSafe safe>
class BASE_PUBLIC RefCountedImpl {
 public:
  DISABLE_COPY_AND_ASSIGN(RefCountedImpl);

 protected:
  RefCountedImpl() noexcept;
  virtual ~RefCountedImpl() noexcept;

 private:
  friend internal::ref_ptr_base<true, safe>;
  friend internal::ref_ptr_base<false, safe>;

  mutable std::conditional_t<safe == ThreadSafe::SAFE, std::atomic<int>, int> refs_;
};

template <bool is_const, ThreadSafe safe>
class ref_ptr_base {
  using type =
      std::conditional_t<is_const, const RefCountedImpl<safe>, RefCountedImpl<safe>>;

 protected:
  ref_ptr_base() noexcept;
  explicit ref_ptr_base(type* ptr) noexcept;
  ref_ptr_base(const ref_ptr_base& rhs) noexcept;
  ref_ptr_base(ref_ptr_base&& rhs) noexcept;
  ~ref_ptr_base();

 protected:
  type* ptr_;
};

template <class D>
static constexpr bool supports_ref_counted =
    std::is_base_of_v<RefCountedImpl<ThreadSafe::NOT>, D> &&
    !std::is_base_of_v<RefCountedImpl<ThreadSafe::SAFE>, D>;
template <class D>
static constexpr bool supports_thread_safe_ref_counted =
    std::is_base_of_v<RefCountedImpl<ThreadSafe::SAFE>, D> &&
    !std::is_base_of_v<RefCountedImpl<ThreadSafe::NOT>, D>;

template <class T>
using ref_ptr_base_for =
    ref_ptr_base<std::is_const_v<T>,
                 supports_thread_safe_ref_counted<T> ? ThreadSafe::SAFE
                                                     : ThreadSafe::NOT>;

}  // namespace base::internal

namespace base {

using RefCounted = internal::RefCountedImpl<internal::ThreadSafe::NOT>;
using RefCountedThreadSafe = internal::RefCountedImpl<internal::ThreadSafe::SAFE>;

template <class T>
requires (internal::supports_ref_counted<T> ||
          internal::supports_thread_safe_ref_counted<T>)
class ref_ptr final : public internal::ref_ptr_base_for<T> {
  using base = internal::ref_ptr_base_for<T>;
 public:
  ref_ptr() = default;

  explicit ref_ptr(T* raw) : base(raw) {}

  /* implicit */ ref_ptr(std::nullptr_t) : base() {}

  ref_ptr(const ref_ptr&) = default;
  ref_ptr(ref_ptr&&) = default;

  template <class D> requires std::convertible_to<D*, T*>
  ref_ptr(const ref_ptr<D>& rhs) : base(rhs) {}

  template <class D> requires std::convertible_to<D*, T*>
  ref_ptr(ref_ptr<D>&& rhs) noexcept : base(std::move(rhs)) {}

  ref_ptr& operator=(ref_ptr rhs) noexcept {
    std::swap(base::ptr_, rhs.ptr_);
    return *this;
  }

  ~ref_ptr() noexcept(std::is_nothrow_destructible_v<T>) = default;

  T* operator->() const noexcept { return get(); }

  T* get() const { return static_cast<T*>(base::ptr_); }

  operator bool() const { return bool(base::ptr_); }

  T& operator*() const { return *get(); }

  bool operator==(const ref_ptr& rhs) const { return base::ptr_ == rhs.ptr_; }

  template <class D> requires std::convertible_to<const D*, const T*>
  bool operator==(D* rhs) const {
    return base::ptr_ == rhs;
  }

  bool operator==(std::nullptr_t) const { return base::ptr_ == nullptr; }

  bool operator!=(const ref_ptr& rhs) const { return base::ptr_ != rhs.ptr_; }

  template <class D> requires std::convertible_to<const D*, const T*>
  bool operator!=(D* rhs) const noexcept {
    return base::ptr_ != rhs;
  }

  bool operator!=(std::nullptr_t) const noexcept { return base::ptr_ != nullptr; }
};

template <class T, class... Args>
auto make_ref_ptr(Args&&... args) {
  return ref_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace base
