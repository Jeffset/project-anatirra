#pragma once

#include "cursedui/view.hpp"
#include "cursedui/views/text_view.hpp"

#include <functional>

namespace cursedui::view {

class CURSEDUI_PUBLIC Button : public TextView {
public:
  using on_click_t = std::function<void()>;

  Button() noexcept;
  ~Button() noexcept override;

  void set_on_click(on_click_t callback) noexcept {
    on_click_ = callback;
  }

  GETTER on_click_t on_click() const noexcept { return on_click_; }

  bool focusable() const noexcept override { return true; }

protected:
 void on_mouse_event(const avada::input::MouseEvent& event) override;
 void on_key_event(const avada::input::KeyboardEvent& event) override;

private:
  using TextView::set_multiline;
  using TextView::multiline;

  on_click_t on_click_;
};
}  // namespace cursedui::view
