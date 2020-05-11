// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/write.hpp"

#include "base/exception.hpp"

#include <unistd.h>

namespace avada::internal {

void write_stdout(const char* data, std::size_t count) {
  if (count == 0)
    return;

  long remaining_count = count;
  const char* remaining_data = data;

  while (remaining_count > 0) {
    auto written = ::write(STDOUT_FILENO, remaining_data, remaining_count);
    if (written < 0)
      throw base::system_exception("'write' call failed.");
    remaining_count -= written;
    remaining_data += written;
  }
}

void write_stdout(std::string_view data) {
  write_stdout(data.data(), data.size());
}

}  // namespace avada::internal
