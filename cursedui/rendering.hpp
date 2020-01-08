//
// Created by jeffset on 12/9/19.
//

#ifndef CURSEDUI_RENDERING_HPP
#define CURSEDUI_RENDERING_HPP

#include "base/macro.hpp"
#include "base/ref_ptr.hpp"
#include "cursedui/color.hpp"
#include "cursedui/dim.hpp"

#include <cstdint>
#include <memory>
#include <string_view>

namespace cursedui {
class Context;
}  // namespace cursedui

namespace cursedui::render {

struct BorderStyle;

struct BorderStyles {
  static const BorderStyle Single;
};

struct Box {
  gfx::Rect rect;
  BorderStyle const* style;
};

class ColorPalette {
 public:
  ColorPalette();
  ~ColorPalette();

  base::ref_ptr<Color> obtain_color(ColorDescr color_descr);

  DISABLE_COPY_AND_ASSIGN(ColorPalette);

 private:
  PIMPL(ColorPalette);
};

class render_exception : public std::exception {
 public:
  const char* what() const noexcept override;
};

class Canvas {
 public:
  explicit Canvas(void*);
  ~Canvas();

  void start();

  Canvas& operator<<(wchar_t ch);
  Canvas& operator<<(const wchar_t* str);
  Canvas& operator<<(std::wstring_view str);
  Canvas& operator<<(const gfx::Point& pos);
  Canvas& operator<<(const Box& box);

 public:
  friend class ColorState;
  friend class ColorState2;

  NODISCARD BgColorState set_background_color(Color* color);
  void set_foreground_color(Color* color);

  DISABLE_COPY_AND_ASSIGN(Canvas);

 private:
  static void init_rendering();
  friend class cursedui::Context;

  //  friend class FgColorState;
  //  friend class BgColorState;

 private:
  PIMPL(Canvas);
};

void fill(Canvas& canvas, wchar_t ch, const gfx::Rect& area);

}  // namespace cursedui::render

#endif  // CURSEDUI_RENDERING_HPP
