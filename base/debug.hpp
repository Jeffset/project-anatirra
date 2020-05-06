// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_DEBUG
#define ANATIRRA_BASE_DEBUG

#include <iostream>
#include <sstream>

namespace base::debug {

class StackTrace {
 public:
  StackTrace() noexcept;
  ~StackTrace() noexcept;

  std::string to_string() const noexcept;
  std::string to_string(std::string_view preambula) const noexcept;

  constexpr static size_t MAX_TRACES = 32;

 private:
  std::ostringstream oss_;
};

namespace internal {

class Logger {
 public:
  Logger(const char* file, int line, bool terminate = false) noexcept;
  ~Logger() noexcept;

  std::ostream& stream() { return ss_; }

 private:
  const char* file_;
  int line_;
  bool terminate_after_;
  std::stringstream ss_;
};

struct LogStarterDummy {
  void operator&(std::ostream&) {}
};

}  // namespace internal

#if defined(DEBUG)
#define ENABLE_LOG 1
#else
#define ENABLE_LOG 0
#endif  // defined(DEBUG)

#define CONDITIONAL_LOG_STREAM(condition, stream) \
  !(condition) ? (void)0 : ::base::debug::internal::LogStarterDummy() & (stream)

#define LOG()                        \
  CONDITIONAL_LOG_STREAM(ENABLE_LOG, \
                         ::base::debug::internal::Logger(__FILE__, __LINE__).stream())

#define ASSERT(condition)                                                \
  CONDITIONAL_LOG_STREAM(                                                \
      ENABLE_LOG && !(condition),                                        \
      ::base::debug::internal::Logger(__FILE__, __LINE__, true).stream() \
          << "Assertion `" #condition "` failed: ")

}  // namespace base::debug

namespace std {

std::ostream& operator<<(std::ostream& out, const wchar_t* wstr);

inline std::ostream& operator<<(std::ostream& out, const std::wstring& wstr) {
  return out << wstr.c_str();
}

}  // namespace std

#endif  // ANATIRRA_BASE_DEBUG
