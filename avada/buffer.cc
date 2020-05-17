// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/buffer.hpp"

#include "avada/write.hpp"
#include "base/debug/debug.hpp"
#include "base/debug/tracing.hpp"
#include "base/exception.hpp"
#include "base/macro.hpp"
#include "base/util.hpp"

#include <algorithm>
#include <cmath>
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
  Renderer() noexcept : output_(std::ios::in | std::ios::out), rle_state_{} {}

  void add(int i, int j, const Buffer::Cell& cell) noexcept {
    // Handle position
    if (position_ != std::pair{i, j - 1}) {
      flush_rle_sequence();

      do {
        if (LIKELY(position_)) {
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
      ScopedModeChange mode_change{*this};

      // Background color
      const auto cell_bg_color = cell.bg_color();
      if (UNLIKELY(bg_color_ != cell_bg_color)) {
        auto encoder = BgColorEncodeVisitor{mode_change};
        std::visit(encoder, cell_bg_color);

        bg_color_ = cell_bg_color;
      }

      if (cell.data().empty() || cell.data()[0] == ' ') {
        // We've only handled background color change, no need to check further.
        // WARNING: this should be changed if INVERSE attribute is ever supported.
        empty_contents = true;
        break;
      }

      {  // Foreground color
        const auto cell_fg_color = alpha_blend(cell.fg_color(), cell_bg_color);
        if (UNLIKELY(fg_color_ != cell_fg_color)) {
          auto encoder = FgColorEncodeVisitor{mode_change};
          std::visit(encoder, cell_fg_color);
          fg_color_ = cell_fg_color;
        }
      }

      const auto cell_attr = cell.attributes();
      auto attr_diff = attributes_.value_or(~cell_attr) ^ cell_attr;
      if (attr_diff & RenderAttributes::BOLD)
        mode_change << (cell_attr & RenderAttributes::BOLD ? 1 : 22);
      if (attr_diff & RenderAttributes::ITALIC)
        mode_change << (cell_attr & RenderAttributes::ITALIC ? 3 : 23);
      if (attr_diff & RenderAttributes::UNDERLINE)
        mode_change << (cell_attr & RenderAttributes::UNDERLINE ? 4 : 24);
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
    internal::write_stdout(code);
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
  friend class ScopedModeChange;

  class ScopedModeChange {
   public:
    ScopedModeChange(Renderer& self) noexcept
        : self_(self), mode_change_started_(false) {}

    ScopedModeChange& operator<<(int arg) noexcept {
      if (UNLIKELY(!mode_change_started_)) {
        self_.flush_rle_sequence();

        self_.output_ << CSI << arg;
        mode_change_started_ = true;
      } else {
        self_.output_ << ';' << arg;
      }
      return *this;
    }

    ~ScopedModeChange() noexcept {
      if (UNLIKELY(mode_change_started_)) {
        self_.output_ << "m";
      }
    }

    DISABLE_COPY_MOVE(ScopedModeChange);

   private:
    Renderer& self_;
    bool mode_change_started_;
  };

  template <bool background>
  struct ColorEncodeVisitor {
    ScopedModeChange& mode_change;

    void operator()(ColorRGB color) const noexcept {
      mode_change << (background ? 48 : 38) << 2 << color.red() << color.green()
                  << color.blue();
    }

    void operator()(SystemColor color) const noexcept {
      int code = static_cast<int>(color) + (background ? 40 : 30);
      if (color == SystemColor::DEFAULT)
        code += 1;
      mode_change << code;
    }
  };

  using FgColorEncodeVisitor = ColorEncodeVisitor<false>;
  using BgColorEncodeVisitor = ColorEncodeVisitor<true>;

  struct RleState {
    std::string_view contents;
    int length;

    RleState() noexcept : contents{}, length(0) {}
    explicit RleState(std::string_view contents) noexcept
        : contents(contents), length(1) {}
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

void Buffer::Cell::blend(const Buffer::Cell& rhs) noexcept {
  // Copy data as is.
  bool changed = data() != rhs.data();
  data_ = rhs.data_;
  data_len_ = rhs.data_len_;

  // Blend colors.
  changed |= fg_color_ != (fg_color_ = alpha_blend(rhs.fg_color(), fg_color_));
  changed |= bg_color_ != (bg_color_ = alpha_blend(rhs.bg_color(), bg_color_));

  // Merge attributes.
  const auto attributes = attributes_ | rhs.attributes();
  changed |= attributes_ != attributes;
  attributes_ = attributes;

  dirty_ |= changed;
}

void Buffer::Cell::assign(const Buffer::Cell& rhs) noexcept {
  if (*this == rhs)
    return;

  *this = rhs;
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
  base::debug::ScopedTrace trace{"Buffer::render"};

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
      cell.clear_dirty();
      renderers[std::pair{cell.fg_color(), cell.bg_color()}].add(i, j, cell);
      screen_reference(i, j) = cell;
    }
  }

  for (int i = rows_with_reference; i < rows_; ++i) {  // Zone "C"
    auto& row = contents_[i];
    for (int j = 0; j < columns_; ++j) {
      auto& cell = row[j];
      cell.clear_dirty();
      renderers[std::pair{cell.fg_color(), cell.bg_color()}].add(i, j, cell);
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
