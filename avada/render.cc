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

#include "avada/render.hpp"
#include "avada/buffer.hpp"
#include "avada/write.hpp"
#include "base/debug/debug.hpp"
#include "base/debug/tracing.hpp"
#include "base/map_util.hpp"

#include <optional>
#include <sstream>
#include <unordered_map>

#define CSI "\x1B["

namespace avada::render {

namespace {

class Renderer {
 public:
  Renderer(const TerminalCapabilities& capabilities) noexcept 
  : output_(std::ios::in | std::ios::out)
  , rle_state_{}
  , capabilities_{&capabilities}
  {}

  void add(int i, int j, const Buffer::Cell& cell) noexcept {
    // Handle position
    if (position_ != std::pair{i, j - 1}) {
      flush_rle_sequence();

      do {
        if (LIKELY(position_)) {
          if (auto [ci, cj] = position_.value(); ci == i) {  // we are on the same row
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

    if (!capabilities_->REP_supported ||
        rle_state_.contents.size() * rle_state_.length < 5) {
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
  const TerminalCapabilities* capabilities_;
};

}  // namespace
 
void render(Buffer& buffer, Buffer& screen_reference, 
            const TerminalCapabilities& capabilities) {
  base::debug::ScopedTrace trace{"Buffer::render"};

  std::unordered_map<std::pair<Color, Color>, Renderer> renderers;
  const auto default_renderer_provider = 
      [&capabilities]() { return Renderer{capabilities}; };

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

  const auto screen_rows = screen_reference.rows();
  const auto screen_columns = screen_reference.columns();
  int rows_with_reference, columns_with_reference;
  auto rows = buffer.rows_, columns = buffer.columns_;
  if (UNLIKELY(screen_rows != rows || screen_columns != columns)) {
    screen_reference = Buffer{rows, columns};
    rows_with_reference = columns_with_reference = 0;
  } else {
    rows_with_reference = screen_rows;
    columns_with_reference = screen_columns;
  }

  int place = 0;
  auto a_b_limit = columns * rows_with_reference;
  while (place < a_b_limit) {
    const auto row_start = place / columns * columns;
    auto a_limit = row_start + columns_with_reference;
    while (place < a_limit) {  // Zone "A"
      if (auto& cell = buffer.contents_[place]; cell.dirty()) {
        cell.clear_dirty();
        auto& ref_cell = screen_reference.contents_[place];
        if (ref_cell == cell) {
          // cell passed screen reference validation, no need to redraw.
          ++place;
          continue;
        }
        const auto i = place / columns;
        const auto j = place % columns;
        base::get_or_default(renderers, std::pair{cell.fg_color(), cell.bg_color()},
                             default_renderer_provider).add(i, j, cell);
        ref_cell = cell;
      }
      ++place;
    }

    auto row_limit = row_start + columns;
    while (place < row_limit) {  // Zone "B"
      auto& cell =  buffer.contents_[place];
      cell.clear_dirty();
      const auto i = place / columns;
      const auto j = place % columns;
      base::get_or_default(renderers, std::pair{cell.fg_color(), cell.bg_color()},
                           default_renderer_provider).add(i, j, cell);
      screen_reference.contents_[place] = cell;
      ++place;
    }
  }

  const auto limit = rows * columns;
  while (place < limit) {
    auto& cell =  buffer.contents_[place];
    cell.clear_dirty();
    const auto i = place / columns;
    const auto j = place % columns;
    base::get_or_default(renderers, std::pair{cell.fg_color(), cell.bg_color()},
                         default_renderer_provider).add(i, j, cell);
    screen_reference.contents_[place] = cell;
    ++place;
  }

  Renderer merged{capabilities};
  LOG() << "Got " << renderers.size() << " renderers";
  for (auto& [_, renderer] : renderers) {
    merged.merge(std::move(renderer));
  }
  merged.do_render();
}

}  // namespace avada::render
