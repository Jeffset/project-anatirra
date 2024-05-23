#include "cursedui/views/button.hpp"
#include "avada/input.hpp"
#include "base/util.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/views/text_view.hpp"

namespace cursedui::view {

using namespace avada::input;

namespace {

constinit const auto kReleaseLMB = MouseEvent::ButtonEvent {
  .code = MouseEvent::Button::LEFT,
  .state = MouseEvent::State::RELEASED,
};

}

Button::Button() noexcept {
  border().set_style(BorderDrawable::Style::SINGLE);
}

Button::~Button() noexcept = default;

void Button::on_mouse_event(const MouseEvent& event) {
  if (!on_click_) 
    return;

  auto is_click = std::visit(base::overloaded {
    [](MouseEvent::ButtonEvent e) { return e == kReleaseLMB; },
    [](auto) { return false; },
  }, event.data);

  if (is_click) {
    on_click_();
  }
}

void Button::on_key_event(const KeyboardEvent& event) {
  if (!on_click_) 
    return;

  auto is_click = std::visit(base::overloaded {
    [](KeyboardKey e) { 
      return e == KeyboardKey::ENTER || e == KeyboardKey::SPACE;
    },
    [](auto){ return false; },
  }, event.key);

  if (is_click) {
    on_click_();
  }
}

}  // namespace cursedui::view
