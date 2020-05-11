// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/color.hpp"

#include "base/util.hpp"

namespace avada::render {

const ColorRGB Colors::TRANSPARENT{0};
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
