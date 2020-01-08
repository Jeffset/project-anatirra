#ifndef NULLABLE_HPP
#define NULLABLE_HPP

#include <cassert>
#include <cstddef>

namespace base {

/**
 * @brief This simple wrapper is all about expressing the nullability of returned pointer.
 * Nothing more, nothing less.
 */
template <class T>
class nullable_ptr {
 public:
  nullable_ptr(std::nullptr_t) noexcept : ptr_(nullptr) {}
  nullable_ptr(T* ptr) : ptr_(ptr) {}

  operator bool() { return ptr_ != nullptr; }

  T* get() noexcept {
    assert(ptr_);
    return ptr_;
  }

 private:
  T* ptr_;
};

}  // namespace base

#endif  // NULLABLE_HPP
