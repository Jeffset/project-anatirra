// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_BUFFER
#define ANATIRRA_AVADA_BUFFER

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace avada::render {

union ColorRGB {
  uint32_t data_;
  alignas(uint32_t) uint8_t rgba_[4];

  inline ColorRGB(uint8_t red, uint8_t green, uint8_t blue)
      : rgba_{red, green, blue, 255} {}
  inline explicit ColorRGB(uint32_t rgb) : data_{rgb} {}
  inline ColorRGB() : data_(0) {}

  inline uint8_t red() const noexcept { return rgba_[0]; }
  inline uint8_t green() const noexcept { return rgba_[1]; }
  inline uint8_t blue() const noexcept { return rgba_[2]; }

  inline uint8_t& red() noexcept { return rgba_[0]; }
  inline uint8_t& green() noexcept { return rgba_[1]; }
  inline uint8_t& blue() noexcept { return rgba_[2]; }

  inline bool operator==(const ColorRGB& rhs) const noexcept {
    return data_ == rhs.data_;
  }
  inline bool operator!=(const ColorRGB& rhs) const noexcept {
    return data_ != rhs.data_;
  }
};

enum class SystemColor : uint8_t {
  BLACK = 30,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  DEFAULT,
};

using Color = std::variant<ColorRGB, SystemColor>;

class Buffer {
 public:
  static constexpr uint8_t ATTRIB_BOLD = 1 << 0;
  static constexpr uint8_t ATTRIB_ITALIC = 1 << 1;
  static constexpr uint8_t ATTRIB_UNDERLINE = 1 << 2;

 public:
  Buffer() noexcept;

  int rows() const noexcept { return rows_; }
  int columns() const noexcept { return columns_; }

  void resize(int rows, int columns) noexcept;
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

    void clear_dirty() noexcept { dirty_ = false; }

   private:
    std::array<char, sizeof(wchar_t)> data_;
    uint8_t data_len_;
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

namespace std {
using namespace avada::render;

template <>
struct hash<Color> {
  size_t operator()(const Color& color) const noexcept {
    return std::visit(Impl{}, color);
  }

  struct Impl {
    size_t operator()(ColorRGB color) const noexcept { return color.data_; }
    size_t operator()(SystemColor color) const noexcept {
      return static_cast<size_t>(color);
    }
  };
};

template <>
struct hash<std::pair<Color, Color>> {
  size_t operator()(const std::pair<Color, Color>&) const noexcept;
};

}  // namespace std

namespace avada::internal {

std::string escape_for_log(std::string code);

}  // namespace avada::internal

#endif  // ANATIRRA_AVADA_BUFFER
