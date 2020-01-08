//
// Created by jeffset on 12/15/19.
//

#include "base/ref_ptr.hpp"

#include <cassert>

namespace base {

RefCounted::RefCounted() noexcept : refs_(0) {}

RefCounted::~RefCounted() noexcept {
  assert(refs_ == 0);
}

namespace internal {

ref_ptr::ref_ptr() noexcept : ptr_(nullptr) {}

ref_ptr::ref_ptr(RefCounted* ptr) noexcept : ptr_(ptr) {
  if (ptr_)
    ++ptr_->refs_;
}

ref_ptr::ref_ptr(std::nullptr_t) noexcept : ptr_(nullptr) {}

ref_ptr::ref_ptr(const ref_ptr& rp) noexcept : ptr_(rp.ptr_) {
  if (ptr_)
    ++ptr_->refs_;
}

ref_ptr::ref_ptr(ref_ptr&& rp) noexcept : ptr_(rp.ptr_) {
  rp.ptr_ = nullptr;
}

ref_ptr& ref_ptr::operator=(const ref_ptr& rp) noexcept {
  if (ptr_ != rp.ptr_) {
    this->~ref_ptr();
    ptr_ = rp.ptr_;
    if (ptr_)
      ++ptr_->refs_;
  }
  return *this;
}

ref_ptr& ref_ptr::operator=(std::nullptr_t) noexcept {
  this->~ref_ptr();
  return *this;
}

ref_ptr& ref_ptr::operator=(ref_ptr&& rp) noexcept {
  if (ptr_ != rp.ptr_) {
    this->~ref_ptr();
    ptr_ = rp.ptr_;
    if (ptr_)
      ++ptr_->refs_;
    rp.ptr_ = nullptr;
  }
  return *this;
}

ref_ptr::~ref_ptr() {
  if (!ptr_) {
    return;
  }
  ptr_->refs_ -= 1;
  if (ptr_->refs_ == 0) {
    delete ptr_;
  }
}

int ref_ptr::ref_count() const {
  return ptr_ ? ptr_->refs_ : 0;
}

}  // namespace internal

}  // namespace base
