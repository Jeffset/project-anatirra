// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_DEBUG_LOGGING
#define ANATIRRA_BASE_DEBUG_LOGGING

#include "base/macro.hpp"

#include "base_config.hpp"

#include <sstream>

#if defined(DEBUG)
#define ENABLE_LOG 1
#define ENABLE_TRACE 1
#define ENABLE_ASSERTS 1
#else
#define ENABLE_LOG 0
#define ENABLE_TRACE 1
#define ENABLE_ASSERTS 0
#endif  // defined(DEBUG)

#define CONDITIONAL_LOG_STREAM(condition, stream) \
  !(condition) ? (void)0 : ::base::debug::internal::LogStarterDummy() & (stream)

#define LOG_IMPL(kind)    \
  CONDITIONAL_LOG_STREAM( \
      ENABLE_##kind, ::base::debug::internal::LoggerProxy(__FILE__, __LINE__).stream())

#define LOG() LOG_IMPL(LOG)
#define TRACE() LOG_IMPL(TRACE)

#define ASSERT(condition)                                                     \
  CONDITIONAL_LOG_STREAM(                                                     \
      (ENABLE_ASSERTS) && UNLIKELY(!(condition)),                             \
      ::base::debug::internal::LoggerProxy(__FILE__, __LINE__, true).stream() \
          << "Assertion `" #condition "` failed: ")

namespace base::debug {

class BASE_PUBLIC LoggerBase {
 public:
  virtual void log(std::streambuf* message) noexcept = 0;

 protected:
  ~LoggerBase() noexcept = default;
};

class BASE_PUBLIC LoggerToStdErr : public LoggerBase {
 public:
  void log(std::streambuf* message) noexcept final;
};

// Main function used to setup logging.
BASE_PUBLIC void setup_logging(LoggerBase* logger) noexcept;

namespace internal {

class BASE_PUBLIC LoggerProxy {
 public:
  LoggerProxy(const char* file, int line, bool terminate = false) noexcept;

  ~LoggerProxy() noexcept;

  DISABLE_COPY_MOVE(LoggerProxy);

  std::ostream& stream() { return ss_; }

 private:
  bool terminate_after_;
  std::stringstream ss_;
};

struct LogStarterDummy {
  inline void operator&(std::ostream&) {}
};

}  // namespace internal

}  // namespace base::debug

namespace std {

BASE_PUBLIC
std::ostream& operator<<(std::ostream& out, const wchar_t* wstr);

inline std::ostream& operator<<(std::ostream& out, const std::wstring& wstr) {
  return out << wstr.c_str();
}

}  // namespace std

#endif  // ANATIRRA_BASE_DEBUG_LOGGING
