// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_INPUT
#define ANATIRRA_AVADA_INPUT

#include "base/exception.hpp"

#include "avada_config.hpp"

#include <codecvt>
#include <cstdint>
#include <locale>
#include <map>
#include <optional>
#include <string>
#include <variant>

namespace avada::input {

struct AVADA_PUBLIC ResizeEvent {
  int columns, rows;
};

enum class AVADA_PUBLIC KeyboardKey : uint8_t {
  // clang-format off
  ENTER,
  TAB,
  SPACE,

  BACKSPACE,
  DELETE,
  ESCAPE,
  INSERT,

  HOME,
  END,

  LEFT,
  RIGHT,
  DOWN,
  UP,

  PAGE_UP,
  PAGE_DOWN,

  F1, F2, F3, F4,
  F5, F6, F7, F8, F9, F10, F11, F12,

  FIRE,  // This is what key '5' on a numeric keypad w-out numlock does.
  // clang-format on
};

enum class AVADA_PUBLIC ServiceEvent {
  IDLE,
};

class AVADA_PUBLIC KeyboardEvent {
 public:
  std::variant<wchar_t, KeyboardKey> key;
  uint8_t modifiers;

  constexpr static uint8_t ALT = 0x1;
  constexpr static uint8_t CTRL = 0x2;
  constexpr static uint8_t SHIFT = 0x4;

  KeyboardEvent() noexcept = default;
  KeyboardEvent(wchar_t key_char, uint8_t modifiers = 0x0) noexcept
      : key(key_char), modifiers(modifiers){};
  KeyboardEvent(KeyboardKey key, uint8_t modifiers = 0x0) noexcept
      : key(key), modifiers(modifiers){};

  KeyboardEvent& operator=(KeyboardKey k) noexcept;

  std::wstring to_string() const;

  inline bool alt() const { return modifiers & ALT; }
  inline bool ctrl() const { return modifiers & CTRL; }
  inline bool shift() const { return modifiers & SHIFT; }

  bool operator==(const KeyboardEvent& rhs) const {
    return key == rhs.key && modifiers == rhs.modifiers;
  }
  bool operator!=(const KeyboardEvent& rhs) const {
    return key != rhs.key || modifiers != rhs.modifiers;
  }
};

class AVADA_PUBLIC MouseEvent {
 public:
  int x, y;

  enum class Button : uint8_t {
    LEFT,
    MIDDLE,
    RIGHT,
  };

  enum class State : uint8_t {
    RELEASED = 0,
    PRESSED = 1,
  };

  enum class Scroll : uint8_t { UP, DOWN };

  struct ButtonEvent {
    Button code : 4;
    State state : 1;
  };

  std::variant<std::monostate, ButtonEvent, Scroll> data;

  MouseEvent(int x, int y) : x(x), y(y), data() {}
  MouseEvent(int x, int y, ButtonEvent btn) : x(x), y(y), data(btn) {}
  MouseEvent(int x, int y, Scroll scrl) : x(x), y(y), data(scrl) {}

  std::string to_string() const noexcept;
};

using Event = std::variant<ServiceEvent, ResizeEvent, KeyboardEvent, MouseEvent>;

class AVADA_PUBLIC unparsed_exception : public base::exception {
 public:
  template <class... Args>
  unparsed_exception(Args&&... args) : exception(std::forward<Args>(args)...){};
};

class InputParser {
 public:
  InputParser() noexcept;

  Event parse_event(std::string_view data) /* throws */;

 private:
  std::optional<KeyboardEvent> parse_keyboard_event(std::string_view data) noexcept;
  std::optional<MouseEvent> parse_mouse_event(std::string_view data) noexcept;

 private:
  void init_keymap() noexcept;

 private:
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  std::map<std::string_view, KeyboardEvent> keymap_;
  // TODO: throttle mouse move events.
};

}  // namespace avada::input

#endif  // ANATIRRA_AVADA_INPUT
