// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_RENDERING
#define ANATIRRA_CURSEDUI_RENDERING

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

  NODISCARD base::ref_ptr<Color> obtain_color(ColorDescr color_descr);
  NODISCARD base::ref_ptr<Color> obtain_color(color_id_t id, ColorDescr color_descr);

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

 private:
  PIMPL(Canvas);
};

void fill(Canvas& canvas, wchar_t ch, const gfx::Rect& area);

}  // namespace cursedui::render

#endif  // ANATIRRA_CURSEDUI_RENDERING
