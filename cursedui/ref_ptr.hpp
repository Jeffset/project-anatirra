//
// Created by jeffset on 12/15/19.
//

#ifndef CURSES_DEMO_REF_PTR_HPP
#define CURSES_DEMO_REF_PTR_HPP

#include <type_traits>

#include "macro.hpp"

namespace cursedui::base {

namespace internal {

class ref_ptr;

}

#define REQUIRES(condition) typename = std::enable_if_t<(condition), void>

class RefCounted {
  DISABLE_COPY_AND_ASSIGN(RefCounted);

 protected:
  RefCounted();
  virtual ~RefCounted();

 private:
  friend class internal::ref_ptr;

  short refs_;
};

namespace internal {

class ref_ptr {
 public:
  ref_ptr();
  explicit ref_ptr(RefCounted* ptr);
  ref_ptr(const ref_ptr& rp);
  ref_ptr(ref_ptr&& rp) noexcept;
  ref_ptr& operator=(const ref_ptr& rp);
  ref_ptr& operator=(ref_ptr&& rp) noexcept;
  ~ref_ptr();

  RefCounted* operator->();
  RefCounted const* operator->() const;
  operator bool() const;
  RefCounted& operator*();
  RefCounted const& operator*() const;

 private:
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

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr(const ref_ptr<D>& rp) : base(rp) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr(ref_ptr<D>&& rp) noexcept : base(rp) {}

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr& operator=(const ref_ptr<D>& rp) {
    base::operator=(rp);
    return *this;
  }

  template <class D, REQUIRES(is_compatible_v<D>)>
  ref_ptr& operator=(ref_ptr<D>&& rp) noexcept {
    base::operator=(rp);
    return *this;
  }

  ~ref_ptr() = default;

  T* operator->() { return reinterpret_cast<T*>(base::operator->()); }

  T const* operator->() const {
    return reinterpret_cast<T const*>(base::operator->());
  }

  operator bool() const { return base::operator bool(); }

  T& operator*() { return (T&)base::operator*(); }

  T const& operator*() const { return (T const&)base::operator*(); }

  bool operator==(const ref_ptr& other) {
    return base::operator->() == other.operator->();
  }
};

}  // namespace cursedui::base

#endif  // CURSES_DEMO_REF_PTR_HPP
