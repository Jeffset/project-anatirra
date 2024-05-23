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

#include "cursedui/region.hpp"

#include "base/debug/debug.hpp"

#include <optional>

namespace cursedui::paint {

namespace {

void deintersect(std::list<gfx::Rect>& rs, std::list<gfx::Rect>& wrs) {
  for (auto wri = wrs.begin(); wri != wrs.end(); /* no auto incr */) {
    for (auto ri = rs.begin(); ri != rs.end(); /* no auto incr */) {
      auto& r = *ri;
      auto& wr = *wri;

      if (!r.intersects(wr)) {
        ++ri;
        continue;
      }

      const int config = ((r.left >= wr.left) << 0) | ((r.top >= wr.top) << 1) |
                         ((r.right <= wr.right) << 2) | ((r.bottom <= wr.bottom) << 3);

      gfx::Rect wr2 = wr;
      LOG() << "----> bottom: " << bool(config & 0b1000)
            << ", right: " << bool(config & 0b0100) << ", top: " << bool(config & 0b0010)
            << ", left: " << bool(config & 0b0001);
      // bottom, right, top, left
      switch (config) {
        case 0b0000:  // r fully contains wr => wr is useless
          wri = wrs.erase(wri);
          goto on_wr_erased;
        case 0b0001:  // left
          wr.right = r.left - 1;
          break;
        case 0b0010:  // top
          wr.bottom = r.top - 1;
          break;
        case 0b0011:  // top, left
          wr.right = r.left - 1;
          wr.top = r.top;
          wr2.bottom = r.top - 1;
          wrs.push_back(wr2);
          break;
        case 0b0100:  // right
          wr.left = r.right + 1;
          break;
        case 0b0101:  // right, left
          wr.left = r.right + 1;
          wr2.right = r.left - 1;
          wrs.push_back(wr2);
          break;
        case 0b0110:  // right, top
          wr.left = r.right + 1;
          wr.top = r.top;
          wr2.bottom = r.top - 1;
          wrs.push_back(wr2);
          break;
        case 0b0111:  // right, top, left
          r.top = wr.bottom + 1;
          break;
        case 0b1000:  // bottom
          wr.top = r.bottom + 1;
          break;
        case 0b1001:  // bottom, left
          wr.top = r.bottom + 1;
          wr2.right = r.left - 1;
          wr2.bottom = r.bottom;
          wrs.push_back(wr2);
          break;
        case 0b1010:  // bottom, top
          wr.top = r.bottom + 1;
          wr2.bottom = r.top - 1;
          wrs.push_back(wr2);
          break;
        case 0b1011:  // bottom, top, left
          r.left = wr.right + 1;
          break;
        case 0b1100:  // bottom, right
          wr.top = r.bottom + 1;
          wr2.left = r.right + 1;
          wr2.bottom = r.bottom;
          wrs.push_back(wr2);
          break;
        case 0b1101:  // bottom, right, left
          r.bottom = wr.top - 1;
          break;
        case 0b1110:  // bottom, right, top
          r.right = wr.left - 1;
          break;
        case 0b1111:  // wr fully contains r => r is useless
          ri = rs.erase(ri);
          goto on_r_erased;
      }
      ++ri;
    on_r_erased:
      continue;
    }
    ++wri;
  on_wr_erased:
    continue;
  }

  const auto not_has_area = [](const gfx::Rect& r) { return !r.has_area(); };
  rs.remove_if(not_has_area);
  wrs.remove_if(not_has_area);
}

[[nodiscard]] gfx::Rect do_clip(const gfx::Rect& rect, const gfx::Rect& clip) noexcept {
  return gfx::Rect {
    .left = std::max(clip.left, rect.left),
    .top = std::max(clip.top, rect.top),
    .right = std::min(clip.right, rect.right),
    .bottom = std::min(clip.bottom, rect.bottom),
  };
}

}  // namespace

Region::Region(const gfx::Rect& rect) noexcept : rects_{rect} {}

void Region::add(const gfx::Rect& rect) noexcept {
  std::list<gfx::Rect> working_rects = {rect};
  deintersect(rects_, working_rects);
  for (auto& wr : working_rects)
    rects_.push_back(wr);
}

void Region::add(const Region& region) noexcept {
  std::list<gfx::Rect> working_rects = region.rects_;
  deintersect(rects_, working_rects);
  for (auto& wr : working_rects)
    rects_.push_back(wr);
}

void Region::add(Region&& region) noexcept {
  std::list<gfx::Rect> working_rects = std::move(region.rects_);
  deintersect(rects_, working_rects);
  for (auto& wr : working_rects)
    rects_.push_back(wr);
}

Region Region::clip(const gfx::Rect& rect) const noexcept {
  Region result;
  for (auto& clip : rects_) {
    auto clipped = do_clip(rect, clip);
    if (clipped.has_area())
      result.rects_.push_back(clipped);
  }
  return result;
}

Region Region::clip(const Region& region) const noexcept {
  Region result;
  for (auto& clip : rects_)
    for (auto& rect : region.rects_) {
      auto clipped = do_clip(rect, clip);
      if (clipped.has_area())
        result.rects_.push_back(clipped);
    }
  return result;
}

bool Region::clip(const gfx::Point& point) const noexcept {
  for (auto& clip : rects_)
    if (clip.contains(point))
      return true;
  return false;
}

bool Region::empty() const noexcept {
  return rects_.empty();
}

}  // namespace cursedui::paint
