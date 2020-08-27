// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/weak_ref.hpp"

namespace base::internal {

template <ThreadSafe thread_safe>
weak_ref_base<thread_safe>::weak_ref_base(
    const WeakReferencedImpl<thread_safe>* ptr) noexcept
    : control_block_(ptr ? ptr->control_block() : nullptr) {}

template <ThreadSafe thread_safe>
weak_ref_base<thread_safe>& weak_ref_base<thread_safe>::operator=(
    const WeakReferencedImpl<thread_safe>* ptr) noexcept {
  if (ptr) {
    control_block_ = ptr->control_block();
  } else {
    control_block_ = nullptr;
  }
  return *this;
}

WeakReferencedImpl<ThreadSafe::NOT>::WeakReferencedImpl() noexcept
    : mutable_this_(this) {}

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
        make_ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>>(mutable_this_);
  return control_block_;
}

template class BASE_PUBLIC WeakRefControlBlock<ThreadSafe::NOT>;
template class BASE_PUBLIC WeakRefControlBlock<ThreadSafe::SAFE>;

template class BASE_PUBLIC weak_ref_base<ThreadSafe::NOT>;
template class BASE_PUBLIC weak_ref_base<ThreadSafe::SAFE>;

}  // namespace base::internal
