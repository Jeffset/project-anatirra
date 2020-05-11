// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_WRITE
#define ANATIRRA_AVADA_WRITE

#include <cstdint>
#include <string_view>

namespace avada::internal {

void write_stdout(const char* data, std::size_t count);

void write_stdout(std::string_view data);

template <std::size_t N>
void write_stdout(const char (&data)[N]) {
  write_stdout(data, N);
}

}  // namespace avada::internal

#endif  // ANATIRRA_AVADA_WRITE
