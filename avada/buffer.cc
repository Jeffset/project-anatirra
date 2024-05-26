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

#include "avada/buffer.hpp"

#include "base/debug/debug.hpp"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <unordered_map>

namespace avada::render {

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
  data_len_ = 1;
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

Buffer::Buffer() noexcept : Buffer(0, 0) {}

Buffer::Buffer(int rows, int columns) noexcept
    : rows_(rows), columns_(columns), contents_(rows * columns) {}

void Buffer::clear() noexcept {
  auto limit = rows_ * columns_;
  for (int place = 0; place < limit; ++place) {
    contents_[place] = Cell{};
  }
}

Buffer::Cell& Buffer::operator()(int i, int j) noexcept {
  ASSERT(i >= 0 && i <= rows_) << "i: " << i;
  ASSERT(j >= 0 && j <= columns_) << "j:" << j;
  return contents_[i * columns_ + j];
}

const Buffer::Cell& Buffer::operator()(int i, int j) const noexcept {
  ASSERT(i >= 0 && i <= rows_) << "i: " << i;
  ASSERT(j >= 0 && j <= columns_) << "j:" << j;
  return contents_[i * columns_ + j];
}

}  // namespace avada::render

using namespace avada::render;

size_t std::hash<std::pair<Color, Color>>::operator()(
    const std::pair<Color, Color>& pair) const noexcept {
  std::hash<Color> hasher;
  static_assert(sizeof(size_t) == 4 || sizeof(size_t) == 8);
  if constexpr (sizeof(size_t) == 8) {
    return (hasher(pair.first) << 32) | hasher(pair.second);
  } else {
    return (hasher(pair.first) << 8) ^ hasher(pair.second);
  }
}

namespace avada::internal {

std::string escape_for_log(std::string code) {
  std::replace(std::begin(code), std::end(code), '\x1b', '^');
  return code;
}

}  // namespace avada::internal
