// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_DEBUG_STACK_TRACE
#define ANATIRRA_BASE_DEBUG_STACK_TRACE

#include "base/config.hpp"

#include <sstream>
#include <string>

namespace base::debug {

class BASE_PUBLIC StackTrace {
 public:
  StackTrace() noexcept;

  std::string to_string() const noexcept;
  std::string to_string(std::string_view preambula) const noexcept;

  constexpr static size_t MAX_TRACES = 32;

 private:
  std::ostringstream oss_;
};

}  // namespace base::debug

#endif  // ANATIRRA_BASE_DEBUG_STACK_TRACE
