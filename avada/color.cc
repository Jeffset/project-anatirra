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

#include "avada/color.hpp"

#include "base/util.hpp"
#include <variant>

namespace avada::render {

constinit const ColorRGB Colors::TRANSPARENT{0};
const ColorRGB Colors::BLACK{0, 0, 0};
const ColorRGB Colors::WHITE{255, 255, 255};

inline int clamp255(int value) {
  return base::clamp(value, 0, 255);
}

inline int channel_blend(int src, int dst, int src_a, int dst_a, int out_a) {
  return clamp255((src * src_a + dst * dst_a * (255 - src_a) / 255) / out_a);
}

Color alpha_blend(const Color& source, const Color& destination) noexcept {
  auto blender = base::overloaded{
      [](SystemColor src, auto) -> Color { return src; },
      [](ColorRGB src, SystemColor dst) -> Color {
        if (src.alpha() == 0)
          return dst;
        return src;
      },
      [](ColorRGB src, ColorRGB dst) -> Color {
        if (src.alpha() == 255)
          return src;
        if (src.alpha() == 0)
          return dst;

        auto a = src.alpha() + dst.alpha() * (255 - src.alpha()) / 255;
        dst.red() = channel_blend(src.red(), dst.red(), src.alpha(), dst.alpha(), a);
        dst.green() =
            channel_blend(src.green(), dst.green(), src.alpha(), dst.alpha(), a);
        dst.blue() = channel_blend(src.blue(), dst.blue(), src.alpha(), dst.alpha(), a);
        dst.alpha() = clamp255(a);
        return dst;
      },
  };
  return std::visit(blender, source, destination);
}

}  // namespace avada::render
