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

#include "base/debug/debug.hpp"

#include "base/debug/stack_trace.hpp"

#include <codecvt>
#include <iostream>
#include <thread>
#include <locale>

namespace base::debug {

namespace {

// Lifecycle of this instance is manager by a provider.
LoggerBase* volatile g_logger;

}  // namespace

void setup_logging(LoggerBase* logger) noexcept {
  g_logger = logger;
}

void LoggerToStdErr::log(std::streambuf* message) noexcept {
  std::cerr << message << std::endl;
}

namespace internal {

LoggerProxy::LoggerProxy(const char* file, int line, bool terminate) noexcept
    : terminate_after_(terminate), ss_(std::ios::in | std::ios::out) {
  ss_ << file << ':' << line << " [thread:" << std::this_thread::get_id() << "] ";
}

LoggerProxy::~LoggerProxy() noexcept {
  if (terminate_after_)
    ss_ << '\n' << StackTrace().to_string("Terminating");
  if (g_logger)
    g_logger->log(ss_.rdbuf());
  if (terminate_after_)
    std::abort();
}

}  // namespace internal

}  // namespace base::debug

std::ostream& std::operator<<(std::ostream& out, const wchar_t* wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  return out << utf8_cvt_.to_bytes(wstr);
}
