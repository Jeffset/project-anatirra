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

#include "base/ref_ptr.hpp"

#include "base/debug/debug.hpp"

#include <cassert>
#include <limits>

namespace base::internal {

namespace {
constexpr auto UNDEFINED_REFS_TS = std::numeric_limits<int>::min();
}

template <>
RefCountedImpl<ThreadSafe::SAFE>::RefCountedImpl() noexcept : refs_(UNDEFINED_REFS_TS) {}

template <>
RefCountedImpl<ThreadSafe::NOT>::RefCountedImpl() noexcept : refs_(0) {}

template <ThreadSafe safe>
RefCountedImpl<safe>::~RefCountedImpl() noexcept {
  if constexpr (safe == ThreadSafe::NOT) {
    ASSERT(refs_ == 0);
  } else {
    ASSERT(refs_ <= 0);
  }
}

template <bool is_const, ThreadSafe safe>
ref_ptr_base<is_const, safe>::ref_ptr_base() noexcept : ptr_(nullptr) {}

template <bool is_const, ThreadSafe safe>
ref_ptr_base<is_const, safe>::ref_ptr_base(type* ptr) noexcept : ptr_(ptr) {
  if constexpr (safe == ThreadSafe::SAFE) {
    if (!ptr_)
      return;
    const auto refs = ptr_->refs_.fetch_add(1, std::memory_order_relaxed);
    if (UNLIKELY(refs == UNDEFINED_REFS_TS)) {
      // thread safe version uses special unutialized magic num to not conflict with zero.
      // It's safe to just store 1 here as it is the first ref_ptr for this object and
      // It can't be shared yet.
      ptr_->refs_.store(1, std::memory_order_relaxed);
    } else if (UNLIKELY(refs <= 0)) {
      // We are too late and the object is being deleted right now;
      // This case is only valid when locking a weak_ref.
      ptr_ = nullptr;
    }
  } else {
    if (ptr_) {
      ++ptr_->refs_;
    }
  }
}

template <bool is_const, ThreadSafe safe>
ref_ptr_base<is_const, safe>::ref_ptr_base(const ref_ptr_base& rhs) noexcept
    : ptr_(rhs.ptr_) {
  if (ptr_) {
    if constexpr (safe == ThreadSafe::SAFE) {
      ptr_->refs_.fetch_add(1, std::memory_order_relaxed);
    } else {
      ++ptr_->refs_;
    }
  }
}

template <bool is_const, ThreadSafe safe>
ref_ptr_base<is_const, safe>::ref_ptr_base(ref_ptr_base&& rhs) noexcept : ptr_(rhs.ptr_) {
  rhs.ptr_ = nullptr;
}

template <bool is_const, ThreadSafe safe>
ref_ptr_base<is_const, safe>::~ref_ptr_base() {
  if constexpr (safe == ThreadSafe::SAFE) {
    if (ptr_ && ptr_->refs_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete ptr_;
    }
  } else {
    if (ptr_ && --ptr_->refs_ == 0) {
      delete ptr_;
    }
  }
}

template class BASE_PUBLIC RefCountedImpl<ThreadSafe::NOT>;
template class BASE_PUBLIC RefCountedImpl<ThreadSafe::SAFE>;

template class BASE_PUBLIC ref_ptr_base<true, ThreadSafe::NOT>;
template class BASE_PUBLIC ref_ptr_base<false, ThreadSafe::NOT>;
template class BASE_PUBLIC ref_ptr_base<true, ThreadSafe::SAFE>;
template class BASE_PUBLIC ref_ptr_base<false, ThreadSafe::SAFE>;

}  // namespace base::internal
