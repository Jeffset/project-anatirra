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

template <bool is_const>
ref_ptr_base<is_const>::ref_ptr_base() noexcept : ptr_(nullptr) {}

template <bool is_const>
ref_ptr_base<is_const>::ref_ptr_base(type* ptr) noexcept : ptr_(ptr) {
  if (ptr_)
    ++ptr_->refs_;
}

template <bool is_const>
ref_ptr_base<is_const>::ref_ptr_base(const ref_ptr_base& rhs) noexcept : ptr_(rhs.ptr_) {
  if (ptr_)
    ++ptr_->refs_;
}

template <bool is_const>
ref_ptr_base<is_const>::ref_ptr_base(ref_ptr_base&& rhs) noexcept : ptr_(rhs.ptr_) {
  rhs.ptr_ = nullptr;
}

template <bool is_const>
ref_ptr_base<is_const>::~ref_ptr_base() {
  if (ptr_ && --ptr_->refs_ == 0) {
    delete ptr_;
  }
}

template <bool is_const>
int ref_ptr_base<is_const>::ref_count() const {
  return ptr_ ? ptr_->refs_ : 0;
}

template class BASE_PUBLIC ref_ptr_base<true>;
template class BASE_PUBLIC ref_ptr_base<false>;

}  // namespace internal

}  // namespace base
