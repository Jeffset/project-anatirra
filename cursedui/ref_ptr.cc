//
// Created by jeffset on 12/15/19.
//

#include "ref_ptr.hpp"

#include <cassert>


namespace cursedui::base {

RefCounted::RefCounted() : refs_(0) {}

RefCounted::~RefCounted() {
  assert(refs_ == 0);
}

namespace internal {

ref_ptr::ref_ptr() : ptr_(nullptr) {}

ref_ptr::ref_ptr(RefCounted* ptr) : ptr_(ptr) {
  if (ptr_) ++ptr_->refs_;
}

ref_ptr::ref_ptr(const ref_ptr& rp) : ptr_(rp.ptr_) {
  if (ptr_) ++ptr_->refs_;
}

ref_ptr::ref_ptr(ref_ptr&& rp) noexcept : ptr_(rp.ptr_) {
  rp.ptr_ = nullptr;
}

ref_ptr& ref_ptr::operator=(const ref_ptr& rp) {
  if (ptr_ != rp.ptr_) {
    (void) ~ref_ptr();
    ptr_ = rp.ptr_;
    if (ptr_) ++ptr_->refs_;
  }
  return *this;
}

ref_ptr& ref_ptr::operator=(ref_ptr&& rp) noexcept {
  if (ptr_ != rp.ptr_) {
    (void) ~ref_ptr();
    ptr_ = rp.ptr_;
    if (ptr_) ++ptr_->refs_;
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

RefCounted* ref_ptr::operator->() {
  return ptr_;
}

RefCounted const* ref_ptr::operator->() const {
  return ptr_;
}

ref_ptr::operator bool() const {
  return ptr_;
}

RefCounted& ref_ptr::operator*() {
  return *ptr_;
}

RefCounted const& ref_ptr::operator*() const {
  return *ptr_;
}

}

}