// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_WEAK_REF
#define ANATIRRA_BASE_WEAK_REF

#include "base/ref_ptr.hpp"

#include "base_config.hpp"

namespace base {

class WeakReferenced;
namespace internal {

struct WeakRefControlBlock : public RefCounted {
  WeakReferenced* ptr_;
  explicit WeakRefControlBlock(WeakReferenced* ptr) : ptr_(ptr) {}
};

class weak_ref_base;
}  // namespace internal

class BASE_PUBLIC WeakReferenced {
 protected:
  WeakReferenced() noexcept;
  virtual ~WeakReferenced() noexcept;

  ref_ptr<internal::WeakRefControlBlock> control_block() const noexcept;
  friend class internal::weak_ref_base;

 private:
  WeakReferenced* mutable_this_;
  mutable ref_ptr<internal::WeakRefControlBlock> control_block_;
};

namespace internal {

class BASE_PUBLIC weak_ref_base {
 protected:
  weak_ref_base() noexcept = default;
  weak_ref_base(const WeakReferenced* ptr) noexcept;

  weak_ref_base& operator=(const WeakReferenced* ptr) noexcept;

  base::ref_ptr<WeakRefControlBlock> control_block_;
};

template <class D>
static constexpr bool supports_weak_referenced = std::is_base_of_v<WeakReferenced, D>;

}  // namespace internal

template <class T, REQUIRES(internal::supports_weak_referenced<T>)>
class weak_ref : public internal::weak_ref_base {
  using base = internal::weak_ref_base;

  template <class D>
  static constexpr bool is_compatible_v = std::is_base_of_v<T, D>;

 public:
  weak_ref() noexcept = default;
  ~weak_ref() noexcept = default;
  explicit weak_ref(T* ptr) noexcept : base(ptr) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(const ref_ptr<D>& ptr) noexcept : base(ptr.get()) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(const weak_ref<D>& wp) noexcept : base(wp) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(weak_ref<D>&& wp) noexcept : base(std::move(wp)) {}

  weak_ref& operator=(std::nullptr_t) {
    control_block_ = nullptr;
    return *this;
  }

  operator bool() const noexcept {
    return control_block_ && control_block_->ptr_ != nullptr;
  }

  T* get() const noexcept {
    return control_block_ ? static_cast<T*>(control_block_->ptr_) : nullptr;
  }

  template <REQUIRES(internal::supports_ref_counted<T>)>
  ref_ptr<T> get_ref_ptr() {
    return ref_ptr<T>(this->get());
  }

  bool operator==(const weak_ref& other) const noexcept {
    return control_block_ == other.control_block_;
  }

  bool operator==(std::nullptr_t) const noexcept {
    return control_block_ == nullptr || control_block_->ptr_ == nullptr;
  }

  bool operator!=(const weak_ref& other) const noexcept {
    return control_block_ != other.control_block_;
  }

  bool operator!=(std::nullptr_t) const noexcept {
    return control_block_ != nullptr && control_block_->ptr_ != nullptr;
  }
};

template <class D>
weak_ref(const ref_ptr<D>&) -> weak_ref<D>;

}  // namespace base

#endif  // ANATIRRA_BASE_WEAK_REF
