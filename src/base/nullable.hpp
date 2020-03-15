#ifndef NULLABLE_HPP
#define NULLABLE_HPP

#include "base/macro.hpp"

#include <cassert>

namespace base {

/**
 * @brief This simple wrapper is all about expressing the nullability of returned pointer.
 * Nothing more, nothing less.
 */
template <class T>
class nullable {
 public:
  /* implicit */ nullable(std::nullptr_t) noexcept : ptr_(nullptr) {}
  /* implicit */ nullable(T* ptr) : ptr_(ptr) {}

  operator bool() const noexcept { return ptr_ != nullptr; }

  NONNULL
  inline T* get() noexcept {
    assert(ptr_);
    return ptr_;
  }

  inline T* get_nullable() noexcept { return ptr_; }

  bool operator==(nullable other) const noexcept { return ptr_ == other.ptr_; }

  bool operator!=(nullable other) const noexcept { return ptr_ != other.ptr_; }

  template <class D>
  friend bool operator==(nullable<D> a, D* b);
  template <class D>
  friend bool operator==(D* a, nullable<D> b);
  template <class D>
  friend bool operator!=(nullable<D> a, D* b);
  template <class D>
  friend bool operator!=(D* a, nullable<D> b);

 private:
  T* ptr_;
};

template <class T>
bool operator==(nullable<T> a, T* b) {
  return a.ptr_ == b;
}

template <class T>
bool operator==(T* a, nullable<T> b) {
  return a == b.ptr_;
}

template <class T>
bool operator!=(nullable<T> a, T* b) {
  return a.ptr_ != b;
}

template <class T>
bool operator!=(T* a, nullable<T> b) {
  return a != b.ptr_;
}

}  // namespace base

#endif  // NULLABLE_HPP
