// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/buffer.hpp"

#include "base/debug.hpp"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <optional>
#include <unistd.h>

#define CSI "\x1B["

namespace avada::render {

Buffer::Cell::Cell() noexcept
    : data_(),
      fg_color_{255, 255, 255},
      bg_color_{0, 0, 0},
      attributes_(0x0),
      dirty_(true) {}

bool Buffer::Cell::operator==(const Buffer::Cell& rhs) const noexcept {
  // dirty flag is ignored here.

  if (data_.size() == 0 && rhs.data_.size() == 0) {
    // If there is nothing to draw in the foreground, then only compare background colors.
    return bg_color_ == rhs.bg_color_;
  }
  return data_ == rhs.data_ && fg_color_ == rhs.fg_color_ && bg_color_ == rhs.bg_color_ &&
         attributes_ == rhs.attributes_;
}

bool Buffer::Cell::operator!=(const Buffer::Cell& rhs) const noexcept {
  // dirty flag is ignored here.
  return !(*this == rhs);  // TODO: maybe optimize this.
}

void Buffer::Cell::set_data(char ch) noexcept {
  if (data_.size() == 1 && data_[0] == ch)
    return;
  data_ = ch;
  dirty_ = true;
}

void Buffer::Cell::set_data(wchar_t wch) noexcept {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  auto chars = utf8_cvt_.to_bytes(wch);
  if (data_ == chars)
    return;
  data_ = chars;
  dirty_ = true;
}

void Buffer::Cell::set_fg_color(Color fg) noexcept {
  if (fg == fg_color_)
    return;
  fg_color_ = fg;
  dirty_ = true;
}

void Buffer::Cell::set_bg_color(Color bg) noexcept {
  if (bg == bg_color_)
    return;
  bg_color_ = bg;
  dirty_ = true;
}

void Buffer::Cell::set_attributes(uint8_t attributes) noexcept {
  if (attributes == attributes_)
    return;
  attributes_ = attributes;
  dirty_ = true;
}

void Buffer::resize(int rows, int columns) noexcept {
  if (columns > column_capacity_) {
    column_capacity_ = columns * 2;
    for (auto& row : contents_) {
      row.resize(column_capacity_);
    }
  }
  columns_ = columns;
  if (rows > row_capacity_) {
    row_capacity_ = rows * 2;
    contents_.resize(row_capacity_, std::vector<Cell>(column_capacity_));
  }
  rows_ = rows;

  // TODO: if extending region, maybe need to clear garbage data out?
}

namespace {

std::string escape_for_log(std::string code) {
  std::replace(std::begin(code), std::end(code), '\x1b', '^');
  return code;
}

class Renderer {
 public:
  Renderer() noexcept = default;

  void add(int i, int j, const Buffer::Cell& cell) noexcept {
    // TODO: Implement optimizations:
    // 1. RLE-like coding.
    // 2. attribute (color) splitting via multiple renderers.

    // Handle position
    if (position_ != std::pair{i, j - 1}) {
      // Change position to target
      output_ << CSI << i + 1 << ';' << j + 1 << 'H';
    }
    position_ = std::pair{i, j};

    bool empty_contents = false;
    do {  // Handle mode
      ModeChange mode_change{output_};

      const auto cell_bg_color = cell.bg_color();
      if (bg_color_ != cell_bg_color) {
        mode_change << 48 << 2 << cell_bg_color.r << cell_bg_color.g << cell_bg_color.b;
        bg_color_ = cell_bg_color;
      }

      if (cell.data().empty() || cell.data()[0] == ' ') {
        // We've only handled background color change, no need to check further.
        // WARNING: this should be changed if INVERSE attribute is supported.
        empty_contents = true;
        break;
      }

      const auto cell_fg_color = cell.fg_color();
      if (fg_color_ != cell_fg_color) {
        mode_change << 38 << 2 << cell_fg_color.r << cell_fg_color.g << cell_fg_color.b;
        fg_color_ = cell_fg_color;
      }
      const auto cell_attr = cell.attributes();
      auto attr_diff = attributes_.value_or(~cell_attr) ^ cell_attr;
      if (attr_diff & Buffer::ATTRIB_BOLD)
        mode_change << (cell_attr & Buffer::ATTRIB_BOLD ? 1 : 22);
      if (attr_diff & Buffer::ATTRIB_ITALIC)
        mode_change << (cell_attr & Buffer::ATTRIB_ITALIC ? 3 : 23);
      if (attr_diff & Buffer::ATTRIB_UNDERLINE)
        mode_change << (cell_attr & Buffer::ATTRIB_UNDERLINE ? 4 : 24);
      attributes_ = cell_attr;
    } while (false);

    // Issue contents
    if (empty_contents)
      output_ << ' ';
    else
      output_ << cell.data();
  }

  void do_render() {
    // Finish generating sequence by putting cursor to (0, 0).
    // NOTE: If this is not done, terminal will try to move our content on resize and
    // we'll be really messed up.
    output_ << CSI "H";

    auto code = output_.str();
    if (code == CSI "H") {
      LOG() << "Nothing to render";
      return;
    }

    LOG() << "Render sequence: " << escape_for_log(code);
    ::write(STDOUT_FILENO, code.data(), code.size());
  }

 private:
  class ModeChange {
   public:
    ModeChange(std::ostream& os) noexcept : os_(os), mode_change_started_(false) {}

    ModeChange& operator<<(int arg) noexcept {
      if (!mode_change_started_) {
        os_ << CSI << arg;
        mode_change_started_ = true;
      } else {
        os_ << ';' << arg;
      }
      return *this;
    }

    ~ModeChange() noexcept {
      if (mode_change_started_) {
        os_ << "m";
      }
    }

   private:
    std::ostream& os_;
    bool mode_change_started_;
  };

 private:
  std::ostringstream output_;
  std::optional<std::pair<int, int>> position_;
  std::optional<Color> bg_color_;
  std::optional<Color> fg_color_;
  std::optional<uint8_t> attributes_;
};

}  // namespace

void Buffer::render(Buffer& screen_reference) {
  Renderer renderer;
  auto screen_rows = screen_reference.rows();
  auto screen_columns = screen_reference.columns();

  screen_reference.resize(rows_, columns_);

  for (int i = 0; i < rows_; ++i) {
    auto& row = contents_[i];
    for (int j = 0; j < columns_; ++j) {
      auto& cell = row[j];
      if (cell.dirty()) {
        if (i < screen_rows && j < screen_columns && screen_reference(i, j) == cell) {
          // cell passed screen reference validation, no need to redraw.
          continue;
        }
        renderer.add(i, j, cell);
        screen_reference(i, j) = cell;
      }
      cell.clear_dirty();
    }
  }
  renderer.do_render();
}

void Buffer::clear() {
  for (int i = 0; i < rows_; ++i) {
    auto& row = contents_[i];
    for (int j = 0; j < columns_; ++j) {
      row[j] = Cell{};
    }
  }
}

Buffer::Cell& Buffer::operator()(int i, int j) noexcept {
  ASSERT(i >= 0 && i <= rows_) << "i: " << i;
  ASSERT(j >= 0 && j <= columns_) << "j:" << j;
  return contents_[i][j];
}

const Buffer::Cell& Buffer::operator()(int i, int j) const noexcept {
  ASSERT(i >= 0 && i <= rows_) << "i: " << i;
  ASSERT(j >= 0 && j <= columns_) << "j:" << j;
  return contents_[i][j];
}
}  // namespace avada::render
