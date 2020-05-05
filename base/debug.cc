// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/debug.hpp"

#include <codecvt>
#include <execinfo.h>
#include <locale>
#include <sstream>

namespace base::debug {

StackTrace::StackTrace() noexcept {
  void* trace_elems[MAX_TRACES];
  trace_count_ = ::backtrace(trace_elems, MAX_TRACES);
  char** stack_syms = ::backtrace_symbols(trace_elems, trace_count_);
  for (auto i = 0u; i < trace_count_; ++i) {
    traces_[i] = stack_syms[i];
  }
}

StackTrace::~StackTrace() noexcept {
  for (auto i = 0u; i < trace_count_; ++i) {
    // FIXME: free memory.
    // delete[] traces_[i];
  }
}

std::string StackTrace::to_string() const noexcept {
  return to_string("");
}

std::string StackTrace::to_string(std::string_view preambula) const noexcept {
  std::ostringstream trace;
  trace << preambula << '\n';
  for (auto i = 0u; i < trace_count_; ++i) {
    trace << '#' << i << ' ' << traces_[i] << '\n';
  }
  return trace.str();
}

namespace internal {

Logger::Logger(const char* file, int line, bool terminate) noexcept
    : file_(file),
      line_(line),
      terminate_after_(terminate),
      ss_(std::ios::in | std::ios::out) {}

Logger::~Logger() noexcept {
  std::cerr << file_ << ':' << line_ << ' ' << ss_.rdbuf() << std::endl;
  if (terminate_after_) {
    std::cerr << StackTrace().to_string("Terminated at:");
    std::terminate();
  }
}

}  // namespace internal

}  // namespace base::debug

std::ostream& std::operator<<(std::ostream& out, const wchar_t* wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  return out << utf8_cvt_.to_bytes(wstr);
}
