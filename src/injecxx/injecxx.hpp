// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_INJECXX_INJECXX
#define ANATIRRA_INJECXX_INJECXX

#include "base/macro.hpp"

namespace injecxx {

namespace detail {

template <class Ret>
class function_lite {
 public:
  template <class Object>
  explicit function_lite(Object* object, Ret (Object::*func)()) noexcept
      : object_(object), fct_((Invoke_t)func) {}

  Ret operator()() noexcept {
    auto* data = (function_lite*)object_;
    return (data->*fct_)();
  }

 private:
  using Invoke_t = Ret (function_lite::*)();

  void* const object_;
  const Invoke_t fct_;
};

}  // namespace detail

template <class T>
class lazy {
 public:
  using type = T;

  inline T& get() noexcept { return getter_(); }

  ~lazy() noexcept = default;

  template <class Object>
  lazy(Object* object, T& (Object::*func)()) noexcept : getter_(object, func) {}

 private:
  detail::function_lite<T&> getter_;
};

}  // namespace injecxx

#endif  // ANATIRRA_INJECXX_INJECXX
