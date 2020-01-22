#ifndef INPUT_HPP
#define INPUT_HPP

#include "cursedui/dim.hpp"

#include <cstdint>
#include <optional>
#include <variant>

namespace cursedui::input {

enum class Key : uint8_t {
  OTHER,

  TAB,
  SPACE,
  ENTER,
  INSERT,
  DELETE,
  BACKSPACE,
  PAGE_UP,
  PAGE_DOWN,
  HOME,
  END,

  LEFT,
  UP,
  RIGHT,
  DOWN,

  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
};

enum class MouseEventCode : uint8_t {
  LEFT_DOWN,
  LEFT_UP,

  RIGHT_DOWN,
  RIGHT_UP,
};

enum class ScrollDirection : uint8_t { UP, DOWN };

struct KeyEvent {
  Key key_code;
  std::optional<wchar_t> key_char;
};

struct MouseEvent {
  gfx::Point location;
  MouseEventCode event_code;

  bool is_mouse_up() const {
    return event_code == MouseEventCode::LEFT_UP ||
           event_code == MouseEventCode::RIGHT_UP;
  }

  bool is_mouse_down() const {
    return event_code == MouseEventCode::LEFT_DOWN ||
           event_code == MouseEventCode::RIGHT_DOWN;
  }
};

struct ScrollEvent {
  gfx::Point location;
  ScrollDirection direction;
};

using InputEvent = std::variant<KeyEvent, MouseEvent, ScrollEvent>;

struct Focusable {
  bool focused;
};

struct NotFocusable {};

using ViewFocus = std::variant<Focusable, NotFocusable>;

}  // namespace cursedui::input

#endif  // INPUT_HPP
