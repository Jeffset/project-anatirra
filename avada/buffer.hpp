// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_BUFFER
#define ANATIRRA_AVADA_BUFFER

#include <cstdint>
#include <string>
#include <vector>

namespace avada::render {

struct Color {
  uint8_t r, g, b;

  bool operator==(const Color& rhs) const noexcept {
    return r == rhs.r && g == rhs.g && b == rhs.b;
  }
  bool operator!=(const Color& rhs) const noexcept {
    return r != rhs.r || g != rhs.g || b != rhs.b;
  }
};

class Buffer {
 public:
  static constexpr uint8_t ATTRIB_BOLD = 1 << 0;
  static constexpr uint8_t ATTRIB_ITALIC = 1 << 1;
  static constexpr uint8_t ATTRIB_UNDERLINE = 1 << 2;

 public:
  int rows() const noexcept { return rows_; }
  int columns() const noexcept { return columns_; }

  void resize(int rows, int columns) noexcept;
  void render(Buffer& screen_reference);
  void clear();

  class Cell {
   public:
    Cell() noexcept;

    bool operator==(const Cell& rhs) const noexcept;
    bool operator!=(const Cell& rhs) const noexcept;

    std::string_view data() const noexcept { return data_; }
    Color fg_color() const noexcept { return fg_color_; }
    Color bg_color() const noexcept { return bg_color_; }
    uint8_t attributes() const noexcept { return attributes_; }
    bool dirty() const noexcept { return dirty_; }

    void set_data(char ch) noexcept;
    void set_data(wchar_t wch) noexcept;
    void set_fg_color(Color fg) noexcept;
    void set_bg_color(Color bg) noexcept;
    void set_attributes(uint8_t attributes) noexcept;

    void clear_dirty() noexcept { dirty_ = false; }

   private:
    std::string data_;
    Color fg_color_;
    Color bg_color_;
    uint8_t attributes_;
    bool dirty_;
  };

  Cell& operator()(int i, int j) noexcept;
  const Cell& operator()(int i, int j) const noexcept;

 private:
  bool check_cell(int i, int j, const Cell& cell) const noexcept {
    if (i >= rows_ || j >= columns_)
      return false;
    return contents_[i][j] == cell;
  }

 private:
  int row_capacity_;
  int column_capacity_;

  int rows_;
  int columns_;

  // cell matrix is stored as a vector of rows.
  std::vector<std::vector<Cell>> contents_;
};

}  // namespace avada::render

#endif  // ANATIRRA_AVADA_BUFFER
