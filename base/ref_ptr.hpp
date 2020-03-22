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
  ref_ptr_base(const ref_ptr_base& rp) noexcept;
  ref_ptr_base(ref_ptr_base&& rp) noexcept;
  ref_ptr_base& operator=(const ref_ptr_base& rp) noexcept;
  ref_ptr_base& operator=(std::nullptr_t) noexcept;
  ref_ptr_base& operator=(ref_ptr_base&& rp) noexcept;
  RefCounted* operator*() noexcept { return ptr_; }
  ~ref_ptr_base();

  int ref_count() const;

 protected:
  RefCounted* ptr_;
};

template <class D>
static constexpr bool supports_ref_counted = std::is_base_of_v<RefCounted, D>;

}  // namespace internal

template <class T, REQUIRES(internal::supports_ref_counted<T>)>
class ref_ptr final : public internal::ref_ptr_base {
  using base = internal::ref_ptr_base;

  template <class D>
  static constexpr bool is_compatible_v = std::is_base_of_v<T, D>;

 public:
  ref_ptr() = default;

  explicit ref_ptr(T* ptr) : base(ptr) {}

  /* implicit */ ref_ptr(std::nullptr_t) : base() {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr(const ref_ptr<D>& rp) : base(rp) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr(ref_ptr<D>&& rp) noexcept : base(std::move(rp)) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr& operator=(const ref_ptr<D>& rp) {
    base::operator=(rp);
    return *this;
  }

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr& operator=(ref_ptr<D>&& rp) noexcept {
    base::operator=(std::move(rp));
    return *this;
  }

  ref_ptr& operator=(std::nullptr_t) {
    base::operator=(nullptr);
    return *this;
  }

  ~ref_ptr() noexcept(std::is_nothrow_destructible_v<T>) = default;

  T* operator->() { return static_cast<T*>(ptr_); }

  T const* operator->() const { return static_cast<T const*>(ptr_); }

  T* get() { return static_cast<T*>(ptr_); }

  T const* get() const { return static_cast<T const*>(ptr_); }

  operator bool() const { return bool(ptr_); }

  T& operator*() { return static_cast<T&>(*ptr_); }

  T const& operator*() const { return static_cast<T const&>(*ptr_); }

  bool operator==(const ref_ptr& other) const { return ptr_ == other.ptr_; }

  template <class D, REQUIRES(is_compatible_v<D>)>
  bool operator==(D* other) const {
    return ptr_ == other;
  }

  bool operator==(std::nullptr_t) const { return ptr_ == nullptr; }

  bool operator!=(const ref_ptr& other) const { return ptr_ != other.ptr_; }

  template <class D, REQUIRES(is_compatible_v<D>)>
  bool operator!=(D* other) const {
    return ptr_ != other;
  }

  bool operator!=(std::nullptr_t) const { return ptr_ != nullptr; }
};

template <class T, class... Args>
auto make_ref_ptr(Args&&... args) {
  return ref_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace base

#endif  // ANATIRRA_BASE_REF_PTR
