// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_DEBUG_TRACING
#define ANATIRRA_BASE_DEBUG_TRACING

#include "base/macro.hpp"

#include <chrono>
#include <string_view>

namespace base::debug {

class ScopedTrace {
 public:
  ScopedTrace(std::string_view name) noexcept;
  ~ScopedTrace() noexcept;

  DISABLE_COPY_MOVE(ScopedTrace);

 private:
  std::string_view name_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}  // namespace base::debug

#endif  // ANATIRRA_BASE_DEBUG_TRACING
