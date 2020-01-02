//
// Created by jeffset on 12/9/19.
//

#ifndef CURSES_DEMO_TEXT_VIEW_HPP
#define CURSES_DEMO_TEXT_VIEW_HPP

#include "view.hpp"

#include <string>


namespace cursedui::view {

class TextView : public View {
 public:
  void set_text(std::wstring_view str);

  [[nodiscard]]
  std::wstring_view get_text() const {
    return text_;
  }

 protected:
  void on_measure(const MeasureSpec& width_spec,
                  const MeasureSpec& height_spec) override;
  void on_layout() override;
  void on_draw(render::Canvas& canvas) override;

 private:
  [[nodiscard]]
  gfx::dim_t width_() const;
  std::wstring text_;
  gfx::Point text_pos_;
};

}

#endif //CURSES_DEMO_TEXT_VIEW_HPP
