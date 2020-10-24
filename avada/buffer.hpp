// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_BUFFER
#define ANATIRRA_AVADA_BUFFER

#include "avada/color.hpp"
#include "base/nullable.hpp"

#include "avada_config.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace avada::render {

class AVADA_PUBLIC Buffer {
 public:
  Buffer() noexcept;
  Buffer(int rows, int columns) noexcept;

  int rows() const noexcept { return rows_; }
  int columns() const noexcept { return columns_; }

  void render(Buffer& screen_reference);
  void clear();

  class Cell {
   public:
    Cell() noexcept;

    bool operator==(const Cell& rhs) const noexcept;

    std::string_view data() const noexcept {
      return std::string_view{data_.data(), static_cast<size_t>(data_len_)};
    }
    Color fg_color() const noexcept { return fg_color_; }
    Color bg_color() const noexcept { return bg_color_; }
    uint8_t attributes() const noexcept { return attributes_; }
    bool dirty() const noexcept { return dirty_; }

    void set_data(char ch) noexcept;
    void set_data(wchar_t wch) noexcept;
    void set_fg_color(Color fg) noexcept;
    void set_bg_color(Color bg) noexcept;
    void set_attributes(uint8_t attributes) noexcept;

    void blend(const Cell& rhs) noexcept;
    void assign(const Cell& rhs) noexcept;

    void mark_dirty() noexcept { dirty_ = true; }
    void clear_dirty() noexcept { dirty_ = false; }

   private:
    std::array<char, sizeof(wchar_t)> data_;
    uint8_t data_len_;
    Color fg_color_;
    Color bg_color_;
    uint8_t attributes_;
    bool dirty_;  // TODO [perf] Add a dirty region to the buffer.
  };

  Cell& operator()(int i, int j) noexcept;
  const Cell& operator()(int i, int j) const noexcept;

 private:
  int rows_;
  int columns_;

  // cell matrix is stored as a vector of rows.
  std::vector<Cell> contents_;
};

}  // namespace avada::render

namespace std {
using namespace avada::render;

template <>
struct AVADA_PUBLIC hash<Color> {
  size_t operator()(const Color& color) const noexcept {
    return std::visit(Impl{}, color);
  }

  struct AVADA_PRIVATE Impl {
    size_t operator()(ColorRGB color) const noexcept { return color.data_; }
    size_t operator()(SystemColor color) const noexcept {
      return static_cast<size_t>(color);
    }
  };
};

template <>
struct AVADA_PUBLIC hash<std::pair<Color, Color>> {
  size_t operator()(const std::pair<Color, Color>&) const noexcept;
};

}  // namespace std

namespace avada::internal {

std::string escape_for_log(std::string code);

}  // namespace avada::internal

#endif  // ANATIRRA_AVADA_BUFFER
