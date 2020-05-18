// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/input.hpp"

#include "base/debug/debug.hpp"
#include "base/util.hpp"

#include <sstream>

namespace avada::input {

namespace {

int parse_decimal(std::string_view::iterator& i) {
  int num = 0;
  while (*i >= '0' && *i <= '9') {
    num = num * 10 + (*i++ - '0');
  }
  return num;
}

const char* keyboard_key_repr(KeyboardKey key) {
  switch (key) {
    // clang-format off
    case KeyboardKey::ENTER:     return "<ENTER>";
    case KeyboardKey::TAB:       return "<TAB>";
    case KeyboardKey::SPACE:     return "<SPACE>";

    case KeyboardKey::BACKSPACE: return "<BACKSPACE>";
    case KeyboardKey::DELETE:    return "<DEL>";
    case KeyboardKey::ESCAPE:    return "<ESC>";
    case KeyboardKey::INSERT:    return "<INSERT>";

    case KeyboardKey::HOME:      return "<HOME>";
    case KeyboardKey::END:       return "<END>";
    case KeyboardKey::FIRE:      return "<5IRE>";

    case KeyboardKey::LEFT:      return "<LEFT>";
    case KeyboardKey::RIGHT:     return "<RIGHT>";
    case KeyboardKey::DOWN:      return "<DOWN>";
    case KeyboardKey::UP:        return "<UP>";

    case KeyboardKey::PAGE_UP:   return "<PAGE_UP>";
    case KeyboardKey::PAGE_DOWN: return "<PAGE_DOWN>";

    case KeyboardKey::F1:        return "<F1>";
    case KeyboardKey::F2:        return "<F2>";
    case KeyboardKey::F3:        return "<F3>";
    case KeyboardKey::F4:        return "<F4>";
    case KeyboardKey::F5:        return "<F5>";
    case KeyboardKey::F6:        return "<F6>";
    case KeyboardKey::F7:        return "<F7>";
    case KeyboardKey::F8:        return "<F8>";
    case KeyboardKey::F9:        return "<F9>";
    case KeyboardKey::F10:       return "<F10>";
    case KeyboardKey::F11:       return "<F11>";
    case KeyboardKey::F12:       return "<F12>";
      // clang-format on
  }
  return "<?KEY?>";
}

[[noreturn]] void handle_unparsed_input(std::string_view data) /* throws */ {
  // This is debug for unparsed input.
  if (data.size() == 1u) {
    char single = data[0];
    throw unparsed_exception("Unparsed single char: 0x", std::hex, (int)single);
  }  // more than 1 byte
  if (data[0] == 0x1B) {
    std::stringstream oss(std::ios::in | std::ios::out);
    oss << "Unparsed escape seq: ";
    for (char c : data) {
      if (std::isprint(c)) {
        oss << '\'' << c << "' ";
      } else {
        oss << "0x" << std::hex << (int)c << ' ';
      }
    }
    throw unparsed_exception(oss.rdbuf());
  }
  throw unparsed_exception("Unparsed raw input: ", data);
}

}  // namespace

KeyboardEvent& KeyboardEvent::operator=(KeyboardKey k) noexcept {
  key = k;
  modifiers = 0x0;
  return *this;
}

std::wstring KeyboardEvent::to_string() const {
  std::wostringstream oss;
  if (modifiers & CTRL)
    oss << "Ctrl + ";
  if (modifiers & ALT)
    oss << "Alt + ";
  if (modifiers & SHIFT)
    oss << "Shift + ";
  std::visit(base::overloaded{[&oss](wchar_t ch) { oss << ch; },
                              [&oss](KeyboardKey k) { oss << keyboard_key_repr(k); }},
             key);
  return oss.str();
}

std::string MouseEvent::to_string() const noexcept {
  std::ostringstream oss;
  oss << '(' << x << ", " << y << ')';
  std::visit(base::overloaded{
                 [&oss](std::monostate) { oss << " [move]"; },
                 [&oss](ButtonEvent ev) {
                   oss << ' ';
                   switch (ev.code) {
                     case Button::LEFT:
                       oss << "Left";
                       break;
                     case Button::RIGHT:
                       oss << "Right";
                       break;
                     case Button::MIDDLE:
                       oss << "Middle";
                       break;
                   }
                   oss << ": " << (ev.state == State::PRESSED ? "pressed" : "released");
                 },
                 [&oss](Scroll scroll) {
                   oss << ' ' << (scroll == Scroll::UP ? "Scroll Up" : "Scroll Down");
                 },
             },
             data);
  return oss.str();
}

InputParser::InputParser() noexcept {
  init_keymap();
}

Event InputParser::parse_event(std::string_view data) {
  if (auto ko = parse_mouse_event(data); ko.has_value()) {
    return ko.value();
  }

  if (auto ko = parse_keyboard_event(data); ko.has_value()) {
    return ko.value();
  }

  handle_unparsed_input(data);
}

std::optional<KeyboardEvent> InputParser::parse_keyboard_event(
    std::string_view data) noexcept {
  // Try to match by keymap.
  if (auto key = keymap_.find(data); key != keymap_.end()) {
    return key->second;
  }

  if (data.size() == 1) {
    auto ch = data[0];

    if (ch == 0) {
      // Weird, but true.
      return KeyboardEvent{KeyboardKey::SPACE, KeyboardEvent::CTRL};
    }

    if (ch == 0x1f) {
      // Also weird, but true.
      return KeyboardEvent{'/', KeyboardEvent::CTRL};
    }

    // Parse CTRL + <alpha>
    if (std::isprint(ch + 96)) {
      return KeyboardEvent{ch + 96, KeyboardEvent::CTRL};
    }

    auto key = KeyboardEvent{ch};
    if (isalpha(ch)) {
      if (isupper(ch)) {
        // Leave upper register be, yet add SHIFT modifier.
        key.modifiers = KeyboardEvent::SHIFT;
      }
      return key;
    }

    if (isprint(ch)) {
      // Just single regular printable character.
      return key;
    }
  }

  // Try to match escape seq with detailed modifier info.
  if (data.size() > 3 && data[0] == '\x1B') {
    auto it = data.rbegin();
    if (std::isprint(*it++)) {
      auto num = *it++;
      int has_semicolon = static_cast<int>(*it == ';');

      auto copy = std::string{data};
      copy.erase(copy.size() - 2 - has_semicolon, 1 + has_semicolon);
      if (auto key_it = keymap_.find(copy); key_it != keymap_.end()) {
        auto key = key_it->second;
        switch (num) {
          case '2':
            key.modifiers = KeyboardEvent::SHIFT;
            break;
          case '3':
            key.modifiers = KeyboardEvent::ALT;
            break;
          case '4':
            key.modifiers = KeyboardEvent::ALT | KeyboardEvent::SHIFT;
            break;
          case '5':
            key.modifiers = KeyboardEvent::CTRL;
            break;
          case '6':
            key.modifiers = KeyboardEvent::CTRL | KeyboardEvent::SHIFT;
            break;
          case '7':
            key.modifiers = KeyboardEvent::ALT | KeyboardEvent::CTRL;
            break;
          case '8':
            key.modifiers =
                KeyboardEvent::ALT | KeyboardEvent::SHIFT | KeyboardEvent::CTRL;
            break;
        }
        return key;
      }
    }
  }

  // Try to match ALT + <KEY>.
  if (data[0] == '\x1B') {
    auto substr = data;
    substr.remove_prefix(1);
    if (auto maybe_key = parse_keyboard_event(substr); maybe_key.has_value()) {
      // matched
      auto key = maybe_key.value();
      key.modifiers |= KeyboardEvent::ALT;
      return key;
    }

    // Give up for no UTF-8 is possible with escape char.
    return {};
  }

  // Try to convert from UTF-8
  if (data.size() > 1) {
    LOG() << "Try to use utf8 cvt on " << data << " with size " << data.size();
    auto wstr = utf8_cvt_.from_bytes(data.begin(), data.end());
    if (utf8_cvt_.converted() == data.size() && wstr.size() == 1) {
      return KeyboardEvent{wstr[0]};
    }
  }

  // Finally, give up.
  return {};
}

std::optional<input::MouseEvent> InputParser::parse_mouse_event(
    std::string_view data) noexcept {
  using namespace input;
  auto i = data.begin();
  if (*i++ != '\x1B')
    return {};
  if (*i++ != '[')
    return {};
  if (*i++ != '<')
    return {};

  int cb = parse_decimal(i);
  if (*i++ != ';')
    return {};

  int cx = parse_decimal(i);
  if (*i++ != ';')
    return {};

  int cy = parse_decimal(i);
  if (*i != 'm' && *i != 'M')
    return {};

  input::MouseEvent mev{cx - 1, cy - 1};

  if (cb < 32) {
    cb = cb & 0b11;
    // regular button press/release event
    auto state = *i == 'M' ? MouseEvent::State::PRESSED : MouseEvent::State::RELEASED;
    MouseEvent::Button btn;
    switch (cb) {
      case 0:
        btn = MouseEvent::Button::LEFT;
        break;
      case 1:
        btn = MouseEvent::Button::MIDDLE;
        break;
      case 2:
        btn = MouseEvent::Button::RIGHT;
        break;
      default:
        return {};
    }
    mev.data = MouseEvent::ButtonEvent{btn, state};
  } else if (cb >= 64) {
    switch (cb) {
      case 64:
        mev.data = MouseEvent::Scroll::UP;
        break;
      case 65:
        mev.data = MouseEvent::Scroll::DOWN;
        break;
      default:
        return {};
    }
  }
  // Or mouse move event

  return mev;
}

void InputParser::init_keymap() noexcept {
  // clang-format off
  keymap_["\x7F"] =      KeyboardKey::BACKSPACE;
  keymap_["\xD"] = keymap_["\x1B" "OM"] =  KeyboardKey::ENTER;
  keymap_["\x1B"] =      KeyboardKey::ESCAPE;
  keymap_["\x1B[2~"] =   KeyboardKey::INSERT;
  keymap_["\x1B[3~"] =   KeyboardKey::DELETE;

  keymap_["\x1B" "Ok"] = '+';
  keymap_["\x1B" "Om"] = '-';
  keymap_["\x1B" "Oj"] = '*';
  keymap_["\x1B" "Oo"] = '/';

  keymap_["\x1B[H"] = keymap_["\x1B[1H"] = KeyboardKey::HOME;
  keymap_["\x1B[F"] = keymap_["\x1B[1F"] = KeyboardKey::END;
  keymap_["\x1B[E"] = keymap_["\x1B[1E"] = KeyboardKey::FIRE;

  keymap_["\x1B[A"] = keymap_["\x1B[1A"] = KeyboardKey::UP;
  keymap_["\x1B[B"] = keymap_["\x1B[1B"] = KeyboardKey::DOWN;
  keymap_["\x1B[C"] = keymap_["\x1B[1C"] = KeyboardKey::RIGHT;
  keymap_["\x1B[D"] = keymap_["\x1B[1D"] = KeyboardKey::LEFT;

  keymap_["\x1B[5~"] =   KeyboardKey::PAGE_UP;
  keymap_["\x1B[6~"] =   KeyboardKey::PAGE_DOWN;

  keymap_["\x1BOP"] = keymap_["\x1B[1P"] = KeyboardKey::F1;
  keymap_["\x1BOQ"] = keymap_["\x1B[1Q"] = KeyboardKey::F2;
  keymap_["\x1BOR"] = keymap_["\x1B[1R"] = KeyboardKey::F3;
  keymap_["\x1BOS"] = keymap_["\x1B[1S"] = KeyboardKey::F4;

  keymap_["\x1B[15~"] =  KeyboardKey::F5;
  keymap_["\x1B[17~"] =  KeyboardKey::F6;
  keymap_["\x1B[18~"] =  KeyboardKey::F7;
  keymap_["\x1B[19~"] =  KeyboardKey::F8;
  keymap_["\x1B[20~"] =  KeyboardKey::F9;
  keymap_["\x1B[21~"] =  KeyboardKey::F10;
  keymap_["\x1B[23~"] =  KeyboardKey::F11;
  keymap_["\x1B[24~"] =  KeyboardKey::F12;

  keymap_["\x9"] =       KeyboardKey::TAB;
  keymap_["\x20"] =      KeyboardKey::SPACE;

  keymap_["\0"] =        KeyboardEvent{KeyboardKey::SPACE, KeyboardEvent::CTRL};
  keymap_["\x1B[Z"] =    KeyboardEvent{KeyboardKey::TAB, KeyboardEvent::SHIFT};
  // clang-format on
}

}  // namespace avada::input
