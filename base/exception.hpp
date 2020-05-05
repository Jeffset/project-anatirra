// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_EXCEPTION
#define ANATIRRA_BASE_EXCEPTION

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace base {

class exception : public std::exception {
 public:
  template <class... Args>
  explicit exception(Args&&... args) : message_(std::forward<Args>(args)...) {
    init();
  }

  const char* what() const noexcept override;

 private:
  void init();

 protected:
  std::string message_;
};

class system_exception : public exception {
 public:
  explicit system_exception(std::string_view message);
};

}  // namespace base

#endif  // ANATIRRA_BASE_EXCEPTION
