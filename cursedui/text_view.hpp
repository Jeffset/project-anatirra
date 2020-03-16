// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_SRC_CURSEDUI_TEXT_VIEW
#define ANATIRRA_SRC_CURSEDUI_TEXT_VIEW

#include "cursedui/view.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace cursedui::render {
class Canvas;
class ColorPalette;
}  // namespace cursedui::render

namespace cursedui::view {

class TextView : public View {
 public:
  TextView();

  void set_text(const std::wstring& str);
  void set_text(std::wstring&& str);

  GETTER std::wstring_view get_text() const { return text_; }

  void set_gravity(gfx::Gravity gravity) noexcept;
  GETTER gfx::Gravity gravity() const noexcept { return gravity_; }

  void set_multiline(bool multiline) noexcept;
  bool multiline() const noexcept { return multiline_; }

  void set_text_color(render::ColorDescr descr) noexcept;
  GETTER render::ColorDescr text_color() const noexcept { return text_color_descr_; }

  bool focusable() const noexcept override { return true; }

 protected:
  void on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;
  void on_colorize(render::ColorPalette& palette) override;

  void on_key_event(const input::KeyEvent& event) override;

  render::BgColorState on_draw(render::Canvas& canvas) override;

 private:
  gfx::Size measure_text(gfx::dim_t max_width, gfx::dim_t max_height) const noexcept;

 private:
  // view attributes:
  std::wstring text_;
  gfx::Gravity gravity_;
  bool multiline_;
  render::ColorDescr text_color_descr_;

  // layout scoped attributes:
  std::vector<std::wstring_view> lines_to_render_;
  gfx::Point text_pos_;
  bool ellipsize_;

  // colors:
  base::ref_ptr<render::Color> text_color_;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_SRC_CURSEDUI_TEXT_VIEW
