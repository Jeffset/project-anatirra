// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/ref_ptr.hpp"

#include <cassert>

namespace base {

RefCounted::RefCounted() noexcept : refs_(0) {}

RefCounted::~RefCounted() noexcept {
  assert(refs_ == 0);
}

namespace internal {

ref_ptr_base::ref_ptr_base() noexcept : ptr_(nullptr) {}

ref_ptr_base::ref_ptr_base(RefCounted* ptr) noexcept : ptr_(ptr) {
  if (ptr_)
    ++ptr_->refs_;
}

ref_ptr_base::ref_ptr_base(const ref_ptr_base& rp) noexcept : ptr_(rp.ptr_) {
  if (ptr_)
    ++ptr_->refs_;
}

ref_ptr_base::ref_ptr_base(ref_ptr_base&& rp) noexcept : ptr_(rp.ptr_) {
  rp.ptr_ = nullptr;
}

ref_ptr_base& ref_ptr_base::operator=(const ref_ptr_base& rp) noexcept {
  if (ptr_ != rp.ptr_) {
    this->~ref_ptr_base();
    ptr_ = rp.ptr_;
    if (ptr_)
      ++ptr_->refs_;
  }
  return *this;
}

ref_ptr_base& ref_ptr_base::operator=(std::nullptr_t) noexcept {
  this->~ref_ptr_base();
  return *this;
}

ref_ptr_base& ref_ptr_base::operator=(ref_ptr_base&& rp) noexcept {
  if (ptr_ != rp.ptr_) {
    this->~ref_ptr_base();
    ptr_ = rp.ptr_;
    if (ptr_)
      ++ptr_->refs_;
    rp.ptr_ = nullptr;
  }
  return *this;
}

ref_ptr_base::~ref_ptr_base() {
  if (!ptr_) {
    return;
  }
  ptr_->refs_ -= 1;
  if (ptr_->refs_ == 0) {
    delete ptr_;
  }
}

int ref_ptr_base::ref_count() const {
  return ptr_ ? ptr_->refs_ : 0;
}

weak_ref_base::weak_ref_base(WeakReferenced* ptr) {
  if (!ptr)
    return;
  control_block_ = ptr->control_block();
}

weak_ref_base::weak_ref_base(const WeakReferenced* ptr) {
  if (!ptr)
    return;
  control_block_ = ptr->control_block();
}

}  // namespace internal

WeakReferenced::WeakReferenced() noexcept : control_block_(nullptr) {}

WeakReferenced::~WeakReferenced() noexcept {
  if (control_block_)
    control_block_->ptr_ = nullptr;
}

ref_ptr<internal::WeakRefControlBlock> WeakReferenced::control_block() noexcept {
  if (!control_block_)
    control_block_ =
        make_ref_ptr<internal::WeakRefControlBlock>(const_cast<WeakReferenced*>(this));
  return control_block_;
}

ref_ptr<internal::WeakRefControlBlock> WeakReferenced::control_block() const noexcept {
  if (!control_block_)
    control_block_ =
        make_ref_ptr<internal::WeakRefControlBlock>(const_cast<WeakReferenced*>(this));
  return control_block_;
}

// namespace internal

}  // namespace base
