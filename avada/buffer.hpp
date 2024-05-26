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

#pragma once

#include "avada/color.hpp"

#include "avada/config.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace avada::render {

struct TerminalCapabilities;

class AVADA_PUBLIC Buffer {
 public:
  Buffer() noexcept;
  Buffer(int rows, int columns) noexcept;

  GETTER int rows() const noexcept { return rows_; }
  GETTER int columns() const noexcept { return columns_; }

  void clear() noexcept;

  class Cell {
   public:
    Cell() noexcept;

    bool operator==(const Cell& rhs) const noexcept;

    GETTER std::string_view data() const noexcept {
      return std::string_view{data_.data(), static_cast<size_t>(data_len_)};
    }
    GETTER Color fg_color() const noexcept { return fg_color_; }
    GETTER Color bg_color() const noexcept { return bg_color_; }
    GETTER uint8_t attributes() const noexcept { return attributes_; }
    GETTER bool dirty() const noexcept { return dirty_; }

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
  friend void render(Buffer&, Buffer&, const TerminalCapabilities&);

  int rows_;
  int columns_;

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
    size_t operator()(ColorRGB color) const noexcept { return color.color_int(); }
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
