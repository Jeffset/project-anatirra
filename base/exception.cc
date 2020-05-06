// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/exception.hpp"

#include "base/debug.hpp"

#include <sstream>

namespace base {

namespace debug {}  // namespace debug

const char* exception::what() const noexcept {
  return message_.c_str();
}

system_exception::system_exception(std::string_view message) : exception(message) {
  std::ostringstream oss;
  oss << "system error: " << std::system_error(errno, std::generic_category()).what()
      << '\n'
      << message_;
  message_ = oss.str();
}

}  // namespace base
