//
// Created by jeffset on 12/9/19.
//

#ifndef CURSEDUI_TEXT_VIEW_HPP
#define CURSEDUI_TEXT_VIEW_HPP

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

 protected:
  void on_measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec) override;
  void on_layout() override;
  void on_colorize(render::ColorPalette& palette) override;
  render::BgColorState on_draw(render::Canvas& canvas) override;

 private:
  GETTER gfx::dim_t text_len_() const;

 private:
  // view attributes:
  std::wstring text_;
  gfx::Gravity gravity_;
  bool multiline_;

  // layout scoped attributes:
  std::vector<std::wstring_view> lines_to_render_;
  gfx::Point text_pos_;
  bool ellipsize_;

  // colors:
  base::ref_ptr<render::Color> text_color_;
};

}  // namespace cursedui::view

#endif  // CURSEDUI_TEXT_VIEW_HPP
