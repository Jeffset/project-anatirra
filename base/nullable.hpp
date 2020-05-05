// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_NULLABLE
#define ANATIRRA_BASE_NULLABLE

#include "base/debug.hpp"
#include "base/macro.hpp"

namespace base {

/**
 * @brief This simple wrapper is all about expressing the nullability of returned pointer.
 * Nothing more, nothing less.
 */
template <class T>
class nullable {
 public:
  /* implicit */ inline nullable(std::nullptr_t) noexcept : ptr_(nullptr) {}
  /* implicit */ inline nullable(T* ptr) : ptr_(ptr) {}

  inline operator bool() const noexcept { return ptr_ != nullptr; }

  NONNULL
  inline T* get() noexcept {
    ASSERT(ptr_);
    return ptr_;
  }

  inline T* get_nullable() noexcept { return ptr_; }

  inline bool operator==(nullable other) const noexcept { return ptr_ == other.ptr_; }

  inline bool operator!=(nullable other) const noexcept { return ptr_ != other.ptr_; }

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
inline bool operator==(nullable<T> a, T* b) {
  return a.ptr_ == b;
}

template <class T>
inline bool operator==(T* a, nullable<T> b) {
  return a == b.ptr_;
}

template <class T>
inline bool operator!=(nullable<T> a, T* b) {
  return a.ptr_ != b;
}

template <class T>
inline bool operator!=(T* a, nullable<T> b) {
  return a != b.ptr_;
}

}  // namespace base

#endif  // ANATIRRA_BASE_NULLABLE
