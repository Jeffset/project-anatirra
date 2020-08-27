// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_WEAK_REF
#define ANATIRRA_BASE_WEAK_REF

#include "base/ref_ptr.hpp"

#include "base_config.hpp"

#include <atomic>

namespace base::internal {

template <ThreadSafe thread_safe>
class WeakReferencedImpl;

template <ThreadSafe thread_safe>
class WeakRefControlBlock : public RefCountedImpl<thread_safe> {
 public:
  explicit WeakRefControlBlock(WeakReferencedImpl<thread_safe>* ptr) noexcept
      : ptr_(ptr) {}

  inline WeakReferencedImpl<thread_safe>* get_ptr() const noexcept {
    if constexpr (thread_safe == ThreadSafe::SAFE) {
      return ptr_.load(std::memory_order_relaxed);
    } else {
      return ptr_;
    }
  }

  inline bool is_ptr_null() const noexcept {
    if constexpr (thread_safe == ThreadSafe::SAFE) {
      return ptr_.load(std::memory_order_relaxed) == nullptr;
    } else {
      return ptr_ == nullptr;
    }
  }

  inline void nullate() noexcept {
    if constexpr (thread_safe == ThreadSafe::SAFE) {
      ptr_.store(nullptr, std::memory_order_relaxed);
    } else {
      ptr_ = nullptr;
    }
  }

 private:
  // clang-format off
  std::conditional_t<thread_safe == ThreadSafe::SAFE,
                     std::atomic<WeakReferencedImpl<ThreadSafe::SAFE>*>,
                     WeakReferencedImpl<ThreadSafe::NOT>*> ptr_;
  // clang-format on
};

template <ThreadSafe thread_safe>
class weak_ref_base;

template <ThreadSafe thread_safe>
class BASE_PUBLIC WeakReferencedImpl;

template <>
class BASE_PUBLIC WeakReferencedImpl<ThreadSafe::SAFE> : public RefCountedThreadSafe {
 protected:
  WeakReferencedImpl() noexcept;
  virtual ~WeakReferencedImpl() noexcept;

  ref_ptr<internal::WeakRefControlBlock<ThreadSafe::SAFE>> control_block()
      const noexcept {
    return control_block_;
  }
  friend class internal::weak_ref_base<ThreadSafe::SAFE>;

 private:
  ref_ptr<internal::WeakRefControlBlock<ThreadSafe::SAFE>> control_block_;
};

template <>
class BASE_PUBLIC WeakReferencedImpl<ThreadSafe::NOT> : public RefCounted {
 protected:
  WeakReferencedImpl() noexcept;
  virtual ~WeakReferencedImpl() noexcept;

  ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>> control_block() const noexcept;
  friend class internal::weak_ref_base<ThreadSafe::NOT>;

 private:
  WeakReferencedImpl* mutable_this_;  // FIXME: get rid of this shit.
  mutable ref_ptr<internal::WeakRefControlBlock<ThreadSafe::NOT>> control_block_;
};

template <ThreadSafe thread_safe>
class BASE_PUBLIC weak_ref_base {
 protected:
  weak_ref_base() noexcept = default;
  weak_ref_base(const WeakReferencedImpl<thread_safe>* ptr) noexcept;

  weak_ref_base& operator=(const WeakReferencedImpl<thread_safe>* ptr) noexcept;

  base::ref_ptr<WeakRefControlBlock<thread_safe>> control_block_;
};

template <class D>
static constexpr bool supports_weak_referenced =
    std::is_base_of_v<WeakReferencedImpl<ThreadSafe::NOT>, D>;

template <class D>
static constexpr bool supports_thread_safe_weak_referenced =
    std::is_base_of_v<WeakReferencedImpl<ThreadSafe::SAFE>, D>;

template <class T>
using weak_ref_base_for =
    weak_ref_base<supports_thread_safe_weak_referenced<T> ? ThreadSafe::SAFE
                                                          : ThreadSafe::NOT>;

}  // namespace base::internal

namespace base {

using WeakReferenced = internal::WeakReferencedImpl<internal::ThreadSafe::NOT>;
using WeakReferencedThreadSafe = internal::WeakReferencedImpl<internal::ThreadSafe::SAFE>;

template <class T>
class weak_ref final : public internal::weak_ref_base_for<T> {
  static_assert(internal::supports_weak_referenced<T> xor
                    internal::supports_thread_safe_weak_referenced<T>,
                "Type must either be WeakReferenced or WeakReferencedThreadSafe");
  using base = internal::weak_ref_base_for<T>;

  template <class D>
  static constexpr bool is_compatible_v = std::is_base_of_v<T, D>;

 public:
  weak_ref() noexcept = default;
  ~weak_ref() noexcept = default;

  template <class D, REQUIRES(is_compatible_v<D>)>
  explicit weak_ref(D* ptr) noexcept : base(ptr) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(const ref_ptr<D>& ptr) noexcept : base(ptr.get()) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(const weak_ref<D>& wp) noexcept : base(wp) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  weak_ref(weak_ref<D>&& wp) noexcept : base(std::move(wp)) {}

  weak_ref& operator=(std::nullptr_t) {
    base::control_block_ = nullptr;
    return *this;
  }

  ref_ptr<T> lock() const {
    T* ptr =
        base::control_block_ ? static_cast<T*>(base::control_block_->get_ptr()) : nullptr;
    return ref_ptr<T>(ptr);
  }

  bool operator==(const weak_ref& other) const noexcept {
    return base::control_block_ == other.control_block_;
  }

  bool operator==(std::nullptr_t) const noexcept {
    return base::control_block_ == nullptr || base::control_block_->is_ptr_null();
  }

  bool operator!=(const weak_ref& other) const noexcept {
    return base::control_block_ != other.control_block_;
  }

  bool operator!=(std::nullptr_t) const noexcept {
    return base::control_block_ != nullptr && !base::control_block_->is_ptr_null();
  }
};

template <class D>
weak_ref(const ref_ptr<D>&) -> weak_ref<D>;

}  // namespace base

namespace std {

template <class T>
struct BASE_PUBLIC hash<base::weak_ref<T>> {
  size_t operator()(const base::weak_ref<T>& ref) const noexcept {
    return std::hash<T*>{}(ref.get());
  }
};

}  // namespace std

#endif  // ANATIRRA_BASE_WEAK_REF
