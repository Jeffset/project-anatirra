// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_AVADA
#define ANATIRRA_AVADA_AVADA

#include "avada/buffer.hpp"
#include "base/exception.hpp"

#include <codecvt>
#include <locale>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct termios;

namespace avada {

namespace input {

struct ResizeEvent {
  int columns, rows;
};

enum class KeyboardKey : uint8_t {
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
  // clang-format on
};

const char* keyboard_key_repr(KeyboardKey key);

enum class ServiceEvent {
  IDLE,
};

class KeyboardEvent {
 public:
  std::variant<wchar_t, KeyboardKey> key;
  uint8_t modifiers;

  constexpr static uint8_t ALT = 0x1;
  constexpr static uint8_t CTRL = 0x2;
  constexpr static uint8_t SHIFT = 0x4;

  KeyboardEvent() = default;
  explicit KeyboardEvent(wchar_t key_char, uint8_t modifiers = 0x0)
      : key(key_char), modifiers(modifiers){};
  explicit KeyboardEvent(KeyboardKey key, uint8_t modifiers = 0x0)
      : key(key), modifiers(modifiers){};

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

  bool operator==(wchar_t ch) const;
  bool operator!=(wchar_t ch) const;
};

class MouseEvent {
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

  std::string to_string() const;
};

using Event = std::variant<ServiceEvent, ResizeEvent, KeyboardEvent, MouseEvent>;

}  // namespace input

class Context {
 public:
  Context();
  ~Context() noexcept;

  input::Event poll_event() /* may throw */;

  void swap_buffers() /* may throw */;

  int get_rows() const noexcept { return rows_; }
  int get_columns() const noexcept { return columns_; }

  render::Buffer& render_buffer() noexcept { return back_buffer_; }
  const render::Buffer& render_buffer() const noexcept { return back_buffer_; }

 private:
  void init_keymap() noexcept;

  void update_size() /* may throw */;

  std::optional<input::KeyboardEvent> parse_keyboard_event(
      std::string_view data) noexcept;
  std::optional<input::MouseEvent> parse_mouse_event(std::string_view data) noexcept;

 private:
  std::unique_ptr<termios> saved_context_;
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cvt_;
  std::map<std::string_view, input::KeyboardEvent> keymap_;

  render::Buffer front_buffer_;
  render::Buffer back_buffer_;

  int rows_;
  int columns_;
};

}  // namespace avada

#endif  // ANATIRRA_AVADA_AVADA
