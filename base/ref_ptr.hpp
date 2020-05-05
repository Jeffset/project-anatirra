// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_REF_PTR
#define ANATIRRA_BASE_REF_PTR

#include "base/macro.hpp"

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace base {

namespace internal {

class ref_ptr_base;

}  // namespace internal

class RefCounted {
 public:
  DISABLE_COPY_AND_ASSIGN(RefCounted);

 protected:
  RefCounted() noexcept;
  virtual ~RefCounted() noexcept;

 private:
  friend class internal::ref_ptr_base;

  int refs_;
};

namespace internal {

class ref_ptr_base {
 public:
  ref_ptr_base() noexcept;
  explicit ref_ptr_base(RefCounted* ptr) noexcept;
  ref_ptr_base(const ref_ptr_base& rhs) noexcept;
  ref_ptr_base(ref_ptr_base&& rhs) noexcept;
  ~ref_ptr_base();

  int ref_count() const;

 protected:
  RefCounted* ptr_;
};

template <class D>
static constexpr bool supports_ref_counted = std::is_base_of_v<RefCounted, D>;

template <class S>
inline auto const_cast_if_needed(S* ptr) {
  if constexpr (std::is_const_v<S>) {
    return const_cast<std::remove_const_t<S>*>(ptr);
  } else {
    return ptr;
  }
}

}  // namespace internal

template <class T, REQUIRES(internal::supports_ref_counted<T>)>
class ref_ptr final : public internal::ref_ptr_base {
  using base = internal::ref_ptr_base;

  template <class D>
  static constexpr bool is_compatible_v = std::is_base_of_v<T, D>;

 public:
  ref_ptr() = default;

  explicit ref_ptr(T* raw) : base(internal::const_cast_if_needed(raw)) {}

  /* implicit */ ref_ptr(std::nullptr_t) : base() {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr(const ref_ptr<D>& rhs) : base(rhs) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr(ref_ptr<D>&& rhs) noexcept : base(std::move(rhs)) {}

  ref_ptr& operator=(ref_ptr rhs) noexcept {
    std::swap(ptr_, rhs.ptr_);
    return *this;
  }

  ~ref_ptr() noexcept(std::is_nothrow_destructible_v<T>) = default;

  T* operator->() const noexcept { return get(); }

  T* get() const { return static_cast<T*>(ptr_); }

  operator bool() const { return bool(ptr_); }

  T& operator*() { return *get(); }

  bool operator==(const ref_ptr& rhs) const { return ptr_ == rhs.ptr_; }

  template <class D, REQUIRES(is_compatible_v<D>)>
  bool operator==(D* rhs) const {
    return ptr_ == rhs;
  }

  bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }

  bool operator!=(const ref_ptr& rhs) const { return ptr_ != rhs.ptr_; }

  template <class D, REQUIRES(is_compatible_v<D>)>
  bool operator!=(D* rhs) const noexcept {
    return ptr_ != rhs;
  }

  bool operator!=(std::nullptr_t) const noexcept { return ptr_ != nullptr; }
};

template <class T, class... Args>
auto make_ref_ptr(Args&&... args) {
  return ref_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace base

#endif  // ANATIRRA_BASE_REF_PTR
