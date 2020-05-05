// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/weak_ref.hpp"

namespace base {

namespace internal {

weak_ref_base::weak_ref_base(WeakReferenced* ptr) noexcept
    : control_block_(ptr ? ptr->control_block() : nullptr) {}

weak_ref_base& weak_ref_base::operator=(WeakReferenced* ptr) noexcept {
  if (ptr) {
    control_block_ = ptr->control_block();
  } else {
    control_block_ = nullptr;
  }
  return *this;
}

}  // namespace internal

WeakReferenced::~WeakReferenced() noexcept {
  if (control_block_)
    control_block_->ptr_ = nullptr;
}

ref_ptr<internal::WeakRefControlBlock> WeakReferenced::control_block() noexcept {
  if (!control_block_)
    control_block_ = make_ref_ptr<internal::WeakRefControlBlock>(this);
  return control_block_;
}

}  // namespace base
