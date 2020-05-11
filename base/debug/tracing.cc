// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/debug/tracing.hpp"

#include "base/debug/debug.hpp"

namespace base::debug {

ScopedTrace::ScopedTrace(std::string_view name) noexcept
    : name_{name}, start_{std::chrono::high_resolution_clock::now()} {}

ScopedTrace::~ScopedTrace() noexcept {
  using namespace std::chrono;
  auto duration = high_resolution_clock::now() - start_;
  TRACE() << "trace (" << name_ << "): " << duration_cast<microseconds>(duration).count()
          << L" μs";
}

}  // namespace base::debug
