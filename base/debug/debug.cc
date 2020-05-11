// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/debug/debug.hpp"

#include "base/debug/stack_trace.hpp"

#include <codecvt>
#include <iostream>
#include <locale>

namespace base::debug {

namespace {

// Lifecycle of this instance is manager by a provider.
LoggerBase* g_logger;

}  // namespace

LoggerBase::~LoggerBase() noexcept = default;

void setup_logging(LoggerBase* logger) noexcept {
  g_logger = logger;
}

void LoggerToStdErr::log(std::streambuf* message) noexcept {
  std::cerr << message << std::endl;
}

namespace internal {

LoggerProxy::LoggerProxy(const char* file, int line, bool terminate) noexcept
    : terminate_after_(terminate), ss_(std::ios::in | std::ios::out) {
  ss_ << file << ':' << line << ' ';
}

LoggerProxy::~LoggerProxy() noexcept {
  if (terminate_after_)
    ss_ << '\n' << StackTrace().to_string("Terminating");
  if (g_logger)
    g_logger->log(ss_.rdbuf());
  if (terminate_after_)
    std::terminate();
}

}  // namespace internal

// namespace internal

}  // namespace base::debug

std::ostream& std::operator<<(std::ostream& out, const wchar_t* wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  return out << utf8_cvt_.to_bytes(wstr);
}
