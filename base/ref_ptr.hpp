//
// Created by jeffset on 12/15/19.
//

#ifndef BASE_REF_PTR_HPP
#define BASE_REF_PTR_HPP

#include "base/macro.hpp"

#include <functional>
#include <type_traits>
#include <utility>

namespace base {

namespace internal {

class ref_ptr;
class weak_ptr;

}  // namespace internal

class RefCounted {
 public:
  DISABLE_COPY_AND_ASSIGN(RefCounted);

 protected:
  RefCounted() noexcept;
  virtual ~RefCounted() noexcept;

 private:
  friend class internal::ref_ptr;

  int refs_;
};

namespace internal {

class ref_ptr {
 public:
  ref_ptr() noexcept;
  explicit ref_ptr(RefCounted* ptr) noexcept;
  explicit ref_ptr(std::nullptr_t) noexcept;
  ref_ptr(const ref_ptr& rp) noexcept;
  ref_ptr(ref_ptr&& rp) noexcept;
  ref_ptr& operator=(const ref_ptr& rp) noexcept;
  ref_ptr& operator=(std::nullptr_t) noexcept;
  ref_ptr& operator=(ref_ptr&& rp) noexcept;
  RefCounted* operator*() noexcept { return ptr_; }
  ~ref_ptr();

  int ref_count() const;

 protected:
  RefCounted* ptr_;
};

template <class D>
static constexpr bool supports_ref_counted = std::is_base_of_v<RefCounted, D>;

}  // namespace internal

template <class T, REQUIRES(internal::supports_ref_counted<T>)>
class ref_ptr final : public internal::ref_ptr {
  using base = internal::ref_ptr;

  template <class D>
  static constexpr bool is_compatible_v = std::is_base_of_v<T, D>;

 public:
  ref_ptr() = default;

  explicit ref_ptr(T* ptr) : base(ptr) {}

  explicit ref_ptr(std::nullptr_t) : base(nullptr) {}

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

  bool operator==(const ref_ptr& other) { return ptr_ == other.ptr_; }

  bool operator!=(const ref_ptr& other) { return ptr_ != other.ptr_; }
};

template <class T, class... Args>
auto make_ref_ptr(Args&&... args) {
  return ref_ptr<T>(new T(std::forward<Args>(args)...));
}

class WeakReferenced;
namespace internal {
struct WeakRefControlBlock : public RefCounted {
  WeakReferenced* ptr_;
  explicit WeakRefControlBlock(WeakReferenced* ptr) : ptr_(ptr) {}
};
class weak_ref;
}  // namespace internal

class WeakReferenced {
 protected:
  WeakReferenced() noexcept : control_block_(new internal::WeakRefControlBlock{this}) {}
  ~WeakReferenced() noexcept { control_block_->ptr_ = nullptr; }

 private:
  friend class internal::weak_ref;
  ref_ptr<internal::WeakRefControlBlock> control_block_;
};

namespace internal {

class weak_ref {
 protected:
  weak_ref() = default;
  weak_ref(WeakReferenced* ptr) {
    if (!ptr)
      return;
    control_block_ = ptr->control_block_;
  }
  weak_ref(const WeakReferenced* ptr) {
    if (!ptr)
      return;
    control_block_ = ptr->control_block_;
  }

  base::ref_ptr<WeakRefControlBlock> control_block_;
};

template <class D>
static constexpr bool supports_weak_referenced = std::is_base_of_v<WeakReferenced, D>;

}  // namespace internal

template <class T, REQUIRES(internal::supports_weak_referenced<T>)>
class weak_ref : public internal::weak_ref {
  using base = internal::weak_ref;

  template <class D>
  static constexpr bool is_compatible_v = std::is_base_of_v<T, D>;

 public:
  weak_ref() = default;
  explicit weak_ref(T* ptr) : base(ptr) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  explicit weak_ref(const ref_ptr<D>& ptr) : base(ptr.get()) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(const weak_ref<D>& wp) : base(wp) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(weak_ref<D>&& wp) : base(std::move(wp)) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref& operator=(const weak_ref<D>& wp) {
    base::operator=(wp);
    return *this;
  }

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref& operator=(weak_ref<D>&& wp) noexcept {
    base::operator=(std::move(wp));
    return *this;
  }

  operator bool() const { return control_block_->ptr_ != nullptr; }

  const T* get() const { return static_cast<const T*>(control_block_->ptr_); }
  T* get() { return static_cast<T*>(control_block_->ptr_); }

  template <REQUIRES(internal::supports_ref_counted<T>)>
  ref_ptr<T> get_ref_ptr() {
    return ref_ptr<T>(this->get());
  }
};

template <class D>
weak_ref(const ref_ptr<D>&)->weak_ref<const D>;

template <class D>
weak_ref(ref_ptr<D>&)->weak_ref<const D>;

}  // namespace base

namespace std {

template <class T>
class hash<base::weak_ref<T>> {
  size_t operator()(const base::weak_ref<T>& wp) {
    std::hash<base::internal::WeakRefControlBlock*> hasher{};
    return hasher(wp.control_block_.get());
  }
};

}  // namespace std

#endif  // BASE_REF_PTR_HPP
