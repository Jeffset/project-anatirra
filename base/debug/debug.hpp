/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "base/macro.hpp"

#include "base/config.hpp"

#include <sstream>

#if !defined(NDEBUG)
#define ENABLE_LOG 1
#define ENABLE_TRACE 1
#define ENABLE_ASSERTS 1
#else
#define ENABLE_LOG 0
#define ENABLE_TRACE 1
#define ENABLE_ASSERTS 0
#endif  // !defined(NDEBUG)

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

class BASE_PUBLIC LoggerToStdErr final : public LoggerBase {
 public:
  void log(std::streambuf* message) noexcept override;
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
