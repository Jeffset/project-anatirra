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

#include "base/ref_ptr.hpp"

#include "base/config.hpp"

#include <atomic>
#include <cstddef>
#include <concepts>

namespace base::internal {

template <ThreadSafe thread_safe>
class WeakReferencedImpl;

template <ThreadSafe thread_safe>
class WeakRefControlBlock : public RefCountedImpl<thread_safe> {
 public:
  explicit WeakRefControlBlock(WeakReferencedImpl<thread_safe>* ptr) noexcept
      : ptr_(ptr) {}

  inline WeakReferencedImpl<thread_safe>* get_ptr() const noexcept {
    if constexpr (thread_safe == ThreadSafe::SAFE) {
      return ptr_.load(std::memory_order_relaxed);
    } else {
      return ptr_;
    }
  }

  inline bool is_ptr_null() const noexcept {
    if constexpr (thread_safe == ThreadSafe::SAFE) {
      return ptr_.load(std::memory_order_relaxed) == nullptr;
    } else {
      return ptr_ == nullptr;
    }
  }

  inline void nullate() noexcept {
    if constexpr (thread_safe == ThreadSafe::SAFE) {
      ptr_.store(nullptr, std::memory_order_relaxed);
    } else {
      ptr_ = nullptr;
    }
  }

 private:
  // clang-format off
  std::conditional_t<thread_safe == ThreadSafe::SAFE,
                     std::atomic<WeakReferencedImpl<ThreadSafe::SAFE>*>,
                     WeakReferencedImpl<ThreadSafe::NOT>*> ptr_;
  // clang-format on
};

template <ThreadSafe thread_safe>
class weak_ref_base;

template <ThreadSafe thread_safe>
class BASE_PUBLIC WeakReferencedImpl;

template <>
class BASE_PUBLIC WeakReferencedImpl<ThreadSafe::SAFE> : public RefCountedThreadSafe {
 protected:
  WeakReferencedImpl() noexcept;
  virtual ~WeakReferencedImpl() noexcept;

  ref_ptr<internal::WeakRefControlBlock<ThreadSafe::SAFE>> control_block()
      const noexcept {
    return control_block_;
  }
  friend class internal::weak_ref_base<ThreadSafe::SAFE>;

 private:
  ref_ptr<internal::WeakRefControlBlock<ThreadSafe::SAFE>> control_block_;
};

template <>
class BASE_PUBLIC WeakReferencedImpl<ThreadSafe::NOT> : public RefCounted {
 protected:
  WeakReferencedImpl() noexcept;
  virtual ~WeakReferencedImpl() noexcept;

  ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>> control_block() const noexcept;
  friend class internal::weak_ref_base<ThreadSafe::NOT>;

 private:
  mutable ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>> control_block_;
};

template <ThreadSafe thread_safe>
class BASE_PUBLIC weak_ref_base {
 protected:
  weak_ref_base() noexcept = default;
  weak_ref_base(const WeakReferencedImpl<thread_safe>* ptr) noexcept;

  void assign(const WeakReferencedImpl<thread_safe>* ptr) noexcept;
  bool equals(const WeakReferencedImpl<thread_safe>* ptr) const noexcept;
  bool equals(std::nullptr_t) const noexcept;

  base::ref_ptr<WeakRefControlBlock<thread_safe>> control_block_;
};

template <class D>
static constexpr bool supports_weak_referenced =
    std::is_base_of_v<WeakReferencedImpl<ThreadSafe::NOT>, D> &&
    !std::is_base_of_v<WeakReferencedImpl<ThreadSafe::SAFE>, D>;

template <class D>
static constexpr bool supports_thread_safe_weak_referenced =
    std::is_base_of_v<WeakReferencedImpl<ThreadSafe::SAFE>, D> &&
    !std::is_base_of_v<WeakReferencedImpl<ThreadSafe::NOT>, D>;

template <class T>
using weak_ref_base_for =
    weak_ref_base<supports_thread_safe_weak_referenced<T> ? ThreadSafe::SAFE
                                                          : ThreadSafe::NOT>;

}  // namespace base::internal

namespace base {

using WeakReferenced = internal::WeakReferencedImpl<internal::ThreadSafe::NOT>;
using WeakReferencedThreadSafe = internal::WeakReferencedImpl<internal::ThreadSafe::SAFE>;

template <class T> 
requires (internal::supports_weak_referenced<T> ||
          internal::supports_thread_safe_weak_referenced<T>)
class weak_ref final : public internal::weak_ref_base_for<T> {
  using base = internal::weak_ref_base_for<T>;
 public:
  weak_ref() noexcept = default;
  ~weak_ref() noexcept = default;

  weak_ref(const weak_ref& wp) noexcept : base(wp) {}
  weak_ref(weak_ref&& wp) noexcept : base(std::move(wp)) {}

  template <class D> requires std::convertible_to<D*, T*>
  explicit weak_ref(D* ptr) noexcept : base(ptr) {}

  template <class D> requires std::convertible_to<D*, T*>
  explicit weak_ref(const ref_ptr<D>& ptr) noexcept : base(ptr.get()) {}

  template <class D> requires std::convertible_to<D*, T*>
  weak_ref(const weak_ref<D>& wp) noexcept : base(wp) {}

  template <class D> requires std::convertible_to<D*, T*>
  weak_ref(weak_ref<D>&& wp) noexcept : base(std::move(wp)) {}

  weak_ref(std::nullptr_t) noexcept : base() {}

  weak_ref& operator=(const weak_ref& wp) noexcept {
    base::control_block_ = wp.control_block_;
    return *this;
  }

  weak_ref& operator=(weak_ref&& wp) noexcept {
    base::control_block_ = std::move(wp.control_block_);
    return *this;
  }

  template <class D> requires std::convertible_to<D*, T*>
  weak_ref& operator=(const weak_ref<D>& wp) noexcept { 
    base::control_block_ = wp.control_block_;
    return *this;
  }

  template <class D> requires std::convertible_to<D*, T*>
  weak_ref& operator=(weak_ref<D>&& wp) noexcept { 
    base::control_block_ = std::move(wp.control_block_);
    return *this;
  }

  template <class D> requires std::convertible_to<D*, T*>
  weak_ref& operator=(const ref_ptr<D>& wp) noexcept { 
    base::assign(wp.get());
    return *this;
  }

  weak_ref& operator=(std::nullptr_t) {
    base::control_block_ = nullptr;
    return *this;
  }

  ref_ptr<T> lock() const {
    auto* ptr = base::control_block_ ?
        static_cast<T*>(base::control_block_->get_ptr()) : nullptr;
    return ref_ptr<T>(ptr);
  }

  bool operator==(const weak_ref& other) const noexcept {
    return base::control_block_ == other.control_block_;
  }

  bool operator!=(const weak_ref& other) const noexcept {
    return base::control_block_ != other.control_block_;
  }

  template <class D>
  requires std::convertible_to<D*, T*> && internal::supports_weak_referenced<T>
  bool operator==(const ref_ptr<D>& ptr) {
    return base::equals(ptr.get());
  }

  template <class D>
  requires std::convertible_to<D*, T*> && internal::supports_weak_referenced<T>
  bool operator!=(const ref_ptr<D>& ptr) {
    return !base::equals(ptr.get());
  }

  bool operator==(std::nullptr_t) const noexcept requires internal::supports_weak_referenced<T> {
    return base::equals(nullptr);
  }

  bool operator!=(std::nullptr_t) const noexcept requires internal::supports_weak_referenced<T> {
    return !base::equals(nullptr);
  }
};

template <class D>
explicit weak_ref(const ref_ptr<D>&) -> weak_ref<D>;

}  // namespace base

namespace std {

template <class T>
struct BASE_PUBLIC hash<base::weak_ref<T>> {
  size_t operator()(const base::weak_ref<T>& ref) const noexcept {
    return std::hash<T*>{}(ref.get());
  }
};

}  // namespace std
