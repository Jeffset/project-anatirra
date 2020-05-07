// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/buffer.hpp"

#include "base/debug.hpp"
#include "base/exception.hpp"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <optional>
#include <unistd.h>
#include <unordered_map>

#define CSI "\x1B["

namespace avada::render {

namespace {

class Renderer {
 public:
  Renderer() noexcept : output_(std::ios::in | std::ios::out), rle_state_{} {
    // output_ << CSI << 0 << 'm';
  }

  void add(int i, int j, const Buffer::Cell& cell) noexcept {
    // Handle position
    if (position_ != std::pair{i, j - 1}) {
      flush_rle_sequence();

      do {
        if (position_) {
          auto [ci, cj] = position_.value();
          if (ci == i) {  // we are on the same row
            // use CUF
            auto distance = j - cj;
            // For now we know, that cells are added only with increasing i or j.
            ASSERT(distance >= 2);
            output_ << CSI;
            if (distance > 2)
              output_ << distance - 1;
            output_ << 'C';
            break;
          }
        }
        // Change position to target
        output_ << CSI << i + 1 << ';' << j + 1 << 'H';
      } while (false);
    }
    position_ = std::pair{i, j};

    bool empty_contents = false;
    do {  // Handle mode
      ModeChange mode_change{*this};

      {  // Background color
        const auto cell_bg_color = cell.bg_color();
        if (bg_color_ != cell_bg_color) {
          auto encoder = BgColorEncoder{mode_change};
          std::visit(encoder, cell_bg_color);

          bg_color_ = cell_bg_color;
        }
      }

      if (cell.data().empty() || cell.data()[0] == ' ') {
        // We've only handled background color change, no need to check further.
        // WARNING: this should be changed if INVERSE attribute is ever supported.
        empty_contents = true;
        break;
      }

      {  // Foreground color
        const auto cell_fg_color = cell.fg_color();
        if (fg_color_ != cell_fg_color) {
          auto encoder = FgColorEncoder{mode_change};
          std::visit(encoder, cell_fg_color);
          fg_color_ = cell_fg_color;
        }
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
    std::string_view contents = empty_contents ? " " : cell.data();
    if (rle_state_.contents == contents) {
      // New contents matches rle sequence, just increment sequence counter.
      ++rle_state_.length;
      // Nothing is rendered for now.
    } else {
      // Rle finished (if any), render it.
      flush_rle_sequence();

      // Setup new rle sequence
      rle_state_ = RleState{contents};
    }
  }

  void merge(Renderer&& renderer) {
    renderer.flush_rle_sequence();

    std::stringstream buffer;
    std::swap(renderer.output_, buffer);
    output_ << buffer.rdbuf();
  }

  void do_render() {
    flush_rle_sequence();

    // Finish generating sequence by putting cursor to (0, 0).
    // NOTE: If this is not done, terminal will try to move our content on resize and
    // we'll be really messed up.
    output_ << CSI "H";

    auto code = output_.str();
    if (code == CSI "H") {
      LOG() << "Nothing to render";
      return;
    }

    LOG() << "Render sequence: " << ::avada::internal::escape_for_log(code);
    LOG() << "Render sequence length: " << code.size();
    const auto result = ::write(STDOUT_FILENO, code.data(), code.size());
    if (result < 0)
      throw base::system_exception("'write' operation failed");
    else if (static_cast<size_t>(result) != code.size())
      throw base::exception("Expected to write ", code.size(),
                            " characters, but written only ", result);
  }

 private:
  void flush_rle_sequence() {
    if (rle_state_.length == 0)
      return;

    if (rle_state_.contents.size() * rle_state_.length < 5) {
      // It's cheaper to repeat character 5 times, for REP sequence is 5 characters long
      // itself.
      for (int c = 0; c < rle_state_.length; ++c)
        output_ << rle_state_.contents;
    } else {
      output_ << rle_state_.contents << CSI << rle_state_.length - 1 << 'b';
    }
    rle_state_ = RleState();
  }

 private:
  friend class ModeChange;

  class ModeChange {
   public:
    ModeChange(Renderer& self) noexcept : self_(self), mode_change_started_(false) {}

    ModeChange& operator<<(int arg) noexcept {
      if (!mode_change_started_) {
        self_.flush_rle_sequence();

        self_.output_ << CSI << arg;
        mode_change_started_ = true;
      } else {
        self_.output_ << ';' << arg;
      }
      return *this;
    }

    ~ModeChange() noexcept {
      if (mode_change_started_) {
        self_.output_ << "m";
      }
    }

   private:
    Renderer& self_;
    bool mode_change_started_;
  };

  template <bool background>
  struct ColorEncoder {
    ModeChange& mode_change;

    void operator()(ColorRGB color) const noexcept {
      mode_change << (background ? 48 : 38) << 2 << color.red() << color.green()
                  << color.blue();
    }

    void operator()(SystemColor color) const noexcept {
      mode_change << 0 << static_cast<int>(color) + (background ? 10 : 0);
    }
  };

  using FgColorEncoder = ColorEncoder<false>;
  using BgColorEncoder = ColorEncoder<true>;

  struct RleState {
    std::string_view contents;
    int length;

    RleState() : contents{""}, length(0) {}
    explicit RleState(std::string_view contents) : contents(contents), length(1) {}
  };

 private:
  std::stringstream output_;
  std::optional<std::pair<int, int>> position_;
  std::optional<Color> bg_color_;
  std::optional<Color> fg_color_;
  std::optional<uint8_t> attributes_;

  RleState rle_state_;
};

}  // namespace

Buffer::Cell::Cell() noexcept
    : data_{0},
      data_len_{0},
      fg_color_{SystemColor::DEFAULT},
      bg_color_{SystemColor::DEFAULT},
      attributes_(0x0),
      dirty_(true) {}

bool Buffer::Cell::operator==(const Buffer::Cell& rhs) const noexcept {
  // dirty flag is ignored here.

  if (data_len_ != rhs.data_len_)
    return false;

  if (data_len_ == 0) {
    // If there is nothing to draw in the foreground, then only compare background colors.
    return bg_color_ == rhs.bg_color_;
  }

  for (int i = 0; i < data_len_; ++i) {
    if (data_[i] != rhs.data_[i])
      return false;
  }

  return fg_color_ == rhs.fg_color_ && bg_color_ == rhs.bg_color_ &&
         attributes_ == rhs.attributes_;
}

void Buffer::Cell::set_data(char ch) noexcept {
  if (data_len_ == 1 && data_[0] == ch) {
    return;
  }
  data_[0] = ch;
  dirty_ = true;
}

void Buffer::Cell::set_data(wchar_t wch) noexcept {
  // TODO: [perf] maybe do not create it each time.
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  auto chars = utf8_cvt_.to_bytes(wch);
  const auto new_size = chars.size();
  ASSERT(new_size <= data_.size());

  if (new_size == data_len_ &&
      std::equal(std::begin(data_), std::begin(data_) + data_len_, std::begin(chars)))
    return;

  std::copy(std::begin(chars), std::end(chars), std::begin(data_));
  data_len_ = new_size;
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

Buffer::Buffer() noexcept
    : row_capacity_(0), column_capacity_(0), rows_(0), columns_(0) {}

void Buffer::resize(int rows, int columns) noexcept {
  // No content manpulation is done here.
  // All rubbish will be dealt with in |render| method.

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
}

void Buffer::render(Buffer& screen_reference) {
  std::unordered_map<std::pair<Color, Color>, Renderer> renderers;
  const auto screen_rows = screen_reference.rows();
  const auto screen_columns = screen_reference.columns();

  // Enlarge example:
  //     _______________
  //    |xxxxxxxxx|     |
  //    |xxx A xxx|  B  |
  //    |xxxxxxxxx|_____|
  //    |               |
  //    |       C       |
  //    |_______________|
  //
  // A - previous screen size, it has valid screen contents which may be referenced.
  // B, C - invalid screen zones, may not be referenced and should be overwritten with
  //        buffer data even if it is not dirty.
  // A+B+C - current buffer size.
  //
  // (shrink is basically the same, with B and C zones are empty, hence their loops will
  // not be entered and whole buffer may be validated.

  screen_reference.resize(rows_, columns_);

  const auto rows_with_reference = std::min(screen_rows, rows_);
  const auto columns_with_reference = std::min(screen_columns, columns_);

  for (int i = 0; i < rows_with_reference; ++i) {
    auto& row = contents_[i];
    for (int j = 0; j < columns_with_reference; ++j)  // Zone "A"
      if (auto& cell = row[j]; cell.dirty()) {
        cell.clear_dirty();
        if (screen_reference(i, j) == cell) {
          // cell passed screen reference validation, no need to redraw.
          continue;
        }
        renderers[std::pair{cell.fg_color(), cell.bg_color()}].add(i, j, cell);
        screen_reference(i, j) = cell;
      }

    for (int j = columns_with_reference; j < columns_; ++j) {  // Zone "B"
      auto& cell = row[j];
      if (cell.dirty()) {
        cell.clear_dirty();
        renderers[std::pair{cell.fg_color(), cell.bg_color()}].add(i, j, cell);
      }
      screen_reference(i, j) = cell;
    }
  }

  for (int i = rows_with_reference; i < rows_; ++i) {  // Zone "C"
    auto& row = contents_[i];
    for (int j = 0; j < columns_; ++j) {
      auto& cell = row[j];
      if (cell.dirty()) {
        cell.clear_dirty();
        renderers[std::pair{cell.fg_color(), cell.bg_color()}].add(i, j, cell);
      }
      screen_reference(i, j) = cell;
    }
  }

  Renderer merged;
  LOG() << "Got " << renderers.size() << " renderers";
  for (auto& [_, renderer] : renderers) {
    merged.merge(std::move(renderer));
  }
  merged.do_render();
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

namespace std {
using namespace avada::render;

size_t hash<pair<Color, Color>>::operator()(
    const pair<Color, Color>& pair) const noexcept {
  hash<Color> hasher;
  static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);
  if constexpr (sizeof(size_t) == 8) {
    return (hasher(pair.first) << 32) | hasher(pair.second);
  } else {
    return (hasher(pair.first) << 8) ^ hasher(pair.second);
  }
}

}  // namespace std

namespace avada::internal {

std::string escape_for_log(std::string code) {
  std::replace(std::begin(code), std::end(code), '\x1b', '^');
  return code;
}

}  // namespace avada::internal
