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

#include "base/weak_ref.hpp"
#include <cstddef>

namespace base::internal {

template <ThreadSafe thread_safe>
weak_ref_base<thread_safe>::weak_ref_base(
    const WeakReferencedImpl<thread_safe>* ptr) noexcept
    : control_block_(ptr ? ptr->control_block() : nullptr) {}

template <ThreadSafe thread_safe>
void weak_ref_base<thread_safe>::assign(
    const WeakReferencedImpl<thread_safe>* ptr) noexcept {
  if (ptr) {
    control_block_ = ptr->control_block();
  } else {
    control_block_ = nullptr;
  }
}

template<>
bool weak_ref_base<ThreadSafe::NOT>::equals(std::nullptr_t) const noexcept {
  return control_block_ == nullptr || control_block_->is_ptr_null();
}

template<>
bool weak_ref_base<ThreadSafe::NOT>::equals(
    const WeakReferencedImpl<ThreadSafe::NOT>* ptr) const noexcept {
  if (!ptr) return equals(nullptr);
  // Otherwise can just compare the control_blocks' ptrs, given the ptr is alive.
  return control_block_ == ptr->control_block();
}

WeakReferencedImpl<ThreadSafe::NOT>::WeakReferencedImpl() noexcept = default;

WeakReferencedImpl<ThreadSafe::SAFE>::WeakReferencedImpl() noexcept
    : control_block_(new WeakRefControlBlock<ThreadSafe::SAFE>(this)) {}

WeakReferencedImpl<ThreadSafe::NOT>::~WeakReferencedImpl() noexcept {
  if (control_block_)
    control_block_->nullate();
}

WeakReferencedImpl<ThreadSafe::SAFE>::~WeakReferencedImpl() noexcept {
  control_block_->nullate();
}

ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>>
WeakReferencedImpl<ThreadSafe::NOT>::control_block() const noexcept {
  if (!control_block_)
    control_block_ =
        make_ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>>(
          const_cast<WeakReferencedImpl*>(this));
  return control_block_;
}

template class BASE_PUBLIC WeakRefControlBlock<ThreadSafe::NOT>;
template class BASE_PUBLIC WeakRefControlBlock<ThreadSafe::SAFE>;

template class BASE_PUBLIC weak_ref_base<ThreadSafe::NOT>;
template class BASE_PUBLIC weak_ref_base<ThreadSafe::SAFE>;

}  // namespace base::internal
