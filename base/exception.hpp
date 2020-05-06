// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_EXCEPTION
#define ANATIRRA_BASE_EXCEPTION

#include "base/debug.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace base {

class exception : public std::exception {
 public:
  template <class... Args>
  explicit exception(Args&&... args)
      : message_{
            static_cast<const std::ostringstream&>((std::ostringstream() << ... << args))
                .str()} {}

  const char* what() const noexcept override;

  const debug::StackTrace& stack_trace() const noexcept { return stack_trace_; }

 private:
  debug::StackTrace stack_trace_;

 protected:
  std::string message_;
};

class system_exception : public exception {
 public:
  explicit system_exception(std::string_view message);
};

}  // namespace base

#endif  // ANATIRRA_BASE_EXCEPTION
