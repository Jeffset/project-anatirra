// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_TEXT_VIEW
#define ANATIRRA_CURSEDUI_TEXT_VIEW

#include "cursedui/view.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace cursedui::view {

class TextView : public View {
 public:
  TextView();

  void set_text(const std::wstring& str);
  void set_text(std::wstring&& str);

  GETTER std::wstring_view get_text() const { return text_; }

  void set_gravity(base::EnumFlags<gfx::Gravity> gravity) noexcept;
  GETTER base::EnumFlags<gfx::Gravity> gravity() const noexcept { return gravity_; }

  void set_multiline(bool multiline) noexcept;
  bool multiline() const noexcept { return multiline_; }

  void set_text_color(avada::render::Color color) noexcept;
  GETTER avada::render::Color text_color() const noexcept { return text_color_; }

  bool focusable() const noexcept override { return true; }

 protected:
  void on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;

  void on_key_event(const avada::input::KeyboardEvent& event) override;

  void on_draw(paint::Canvas& canvas) override;

 private:
  gfx::Size measure_text(gfx::dim_t max_width, gfx::dim_t max_height) const noexcept;

 private:
  // view attributes:
  std::wstring text_;
  base::EnumFlags<gfx::Gravity> gravity_;
  bool multiline_;
  avada::render::Color text_color_;

  // layout scoped attributes:
  std::vector<std::wstring_view> lines_to_render_;
  gfx::Point text_pos_;
  bool ellipsize_;
};

}  // namespace cursedui::view

#endif  // ANATIRRA_CURSEDUI_TEXT_VIEW
