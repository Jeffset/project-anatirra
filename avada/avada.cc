// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/avada.hpp"

#include "base/debug.hpp"
#include "base/macro.hpp"
#include "base/util.hpp"

#include <algorithm>
#include <csignal>
#include <map>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#define ESC_CH "\x1B"

namespace avada {

namespace {

#define SYSTEM_CALL_NON_ZERO(call) \
  if ((call) != 0)                 \
  throw ::base::system_exception(#call)

template <int N>
inline void write_stdout(const char (&data)[N]) {
  ::write(STDOUT_FILENO, data, N);
}

volatile std::sig_atomic_t g_pending_resize = 0;
Context* g_avada_context;

}  // namespace

Context::Context() {
  ASSERT(g_avada_context == nullptr) << "Only one AvadaContext is permitted to exist";
  std::signal(SIGWINCH, [](int) noexcept { g_pending_resize = 1; });

  std::setlocale(LC_ALL, "");

  saved_context_ = std::make_unique<termios>();
  SYSTEM_CALL_NON_ZERO(::tcgetattr(STDIN_FILENO, saved_context_.get()));

  auto raw = *saved_context_;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  // block to have at least a single byte (actually, we use poll to wait)
  raw.c_cc[VMIN] = 1;
  // do not block in terms of time.
  raw.c_cc[VTIME] = 0;

  SYSTEM_CALL_NON_ZERO(::tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw));

  write_stdout(
      "\x1b[?1000;1006;1003;1049;h"  // Mouse mode
      "\x1b[?25l"
      "\x1b[2J"
      "\x1b[H"
      "\x1b%%G"
      "\x1b="
      "");

  init_keymap();

  update_size();

  // Must be the last line.
  g_avada_context = this;
}

Context::~Context() noexcept {
  std::signal(SIGWINCH, SIG_DFL);
  write_stdout(
      "\x1b[?1000;1006;1003;1049;l"
      "\x1b[?25h");
  ::tcsetattr(STDIN_FILENO, TCSAFLUSH, saved_context_.get());
  ASSERT(g_avada_context == this);
  g_avada_context = nullptr;
}

input::Event Context::poll_event() {
  using namespace std::chrono_literals;
  using namespace input;

  pollfd pfd{STDIN_FILENO, POLLIN, 0};

  int poll_result = 0;
  while (poll_result == 0) {
    poll_result = ::poll(&pfd, 1, 1 /*ms*/);
  }
  if (poll_result == -1) {
    if (errno == EINTR && g_pending_resize) {
      // We've caught SIGWINCH, it's ok.
      g_pending_resize = 0;
      update_size();
      return ResizeEvent{columns_, rows_};
    }
    // Otherwise it's not ok.
    throw base::system_exception("'poll' operation failed");
  }

  // Now data is ready, read it.
  char raw_data[128] = {0};
  ssize_t n_read = 0;
  while (n_read <= 0) {
    n_read = ::read(STDIN_FILENO, &raw_data, sizeof(raw_data));
    if (n_read == -1)
      throw base::system_exception("'read' operation failed");
  }

  std::string_view data{raw_data, static_cast<size_t>(n_read)};

  if (auto ko = parse_mouse_event(data); ko.has_value()) {
    return ko.value();
  }

  if (auto ko = parse_keyboard_event(data); ko.has_value()) {
    return ko.value();
  }

  // This is debug for unparsed input.
  if (n_read == 1) {
    char single = data[0];
    LOG() << "Unparsed single char: 0x" << std::hex << (int)single << "\r\n";
  } else {
    // more than 1 byte
    if (data[0] == 0x1B) {
      std::ostringstream oss("Unparsed escape seq: ");
      for (char c : data) {
        if (std::isprint(c)) {
          oss << '\'' << c << "' ";
        } else {
          oss << "0x" << std::hex << (int)c << ' ';
        }
      }
      LOG() << oss.rdbuf();
    } else {
      LOG() << "Unparsed raw input: " << data;
    }
  }
  return ServiceEvent::IDLE;
}

void Context::swap_buffers() {
  back_buffer_.render(front_buffer_);
}

void Context::init_keymap() noexcept {
  using ke_t = input::KeyboardEvent;
  using kk_t = input::KeyboardKey;
  // clang-format off
  keymap_["\x7F"] =         ke_t{kk_t::BACKSPACE};
  keymap_["\xD"] =          ke_t{kk_t::ENTER};
  keymap_[ESC_CH] =         ke_t{kk_t::ESCAPE};
  keymap_[ESC_CH "[2~"] =   ke_t{kk_t::INSERT};
  keymap_[ESC_CH "[3~"] =   ke_t{kk_t::DELETE};

  keymap_[ESC_CH "[H"] =    ke_t{kk_t::HOME};
  keymap_[ESC_CH "[F"] =    ke_t{kk_t::END};

  keymap_[ESC_CH "[A"] =    ke_t{kk_t::UP};
  keymap_[ESC_CH "[B"] =    ke_t{kk_t::DOWN};
  keymap_[ESC_CH "[C"] =    ke_t{kk_t::RIGHT};
  keymap_[ESC_CH "[D"] =    ke_t{kk_t::LEFT};

  keymap_[ESC_CH "[5~"] =   ke_t{kk_t::PAGE_UP};
  keymap_[ESC_CH "[6~"] =   ke_t{kk_t::PAGE_DOWN};

  keymap_[ESC_CH "[6~"] =   ke_t{kk_t::PAGE_DOWN};

  keymap_[ESC_CH "OP"] =    ke_t{kk_t::F1};
  keymap_[ESC_CH "OQ"] =    ke_t{kk_t::F2};
  keymap_[ESC_CH "OR"] =    ke_t{kk_t::F3};
  keymap_[ESC_CH "OS"] =    ke_t{kk_t::F4};

  keymap_[ESC_CH "[1P"] =    ke_t{kk_t::F1};
  keymap_[ESC_CH "[1Q"] =    ke_t{kk_t::F2};
  keymap_[ESC_CH "[1R"] =    ke_t{kk_t::F3};
  keymap_[ESC_CH "[1S"] =    ke_t{kk_t::F4};

  keymap_[ESC_CH "[15~"] =  ke_t{kk_t::F5};
  keymap_[ESC_CH "[17~"] =  ke_t{kk_t::F6};
  keymap_[ESC_CH "[18~"] =  ke_t{kk_t::F7};
  keymap_[ESC_CH "[19~"] =  ke_t{kk_t::F8};
  keymap_[ESC_CH "[20~"] =  ke_t{kk_t::F9};
  keymap_[ESC_CH "[21~"] =  ke_t{kk_t::F10};
  keymap_[ESC_CH "[23~"] =  ke_t{kk_t::F11};
  keymap_[ESC_CH "[24~"] =  ke_t{kk_t::F12};

  keymap_["\x9"] =          ke_t{kk_t::TAB};
  keymap_["\x20"] =         ke_t{kk_t::SPACE};

  keymap_["\0"] =           ke_t{kk_t::SPACE, ke_t::CTRL};
  keymap_[ESC_CH "[Z"] =    ke_t{kk_t::TAB, ke_t::SHIFT};
  // clang-format on
}

void Context::update_size() {
  struct winsize ws;
  SYSTEM_CALL_NON_ZERO(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws));
  rows_ = ws.ws_row;
  columns_ = ws.ws_col;
  back_buffer_.resize(rows_, columns_);
}

std::optional<input::KeyboardEvent> Context::parse_keyboard_event(
    std::string_view data) noexcept {
  using ke_t = input::KeyboardEvent;
  // Try to match by keymap.
  if (auto key = keymap_.find(data); key != keymap_.end()) {
    return key->second;
  }

  if (data.size() == 1) {
    auto ch = data[0];

    if (ch == 0) {
      // Weird, but true.
      return ke_t{input::KeyboardKey::SPACE, ke_t::CTRL};
    }

    // Parse CTRL + <alpha>
    if (ch >= 1 && ch <= 26) {
      return ke_t{ch + 96, ke_t::CTRL};
    }

    auto key = ke_t{ch};
    if (isalpha(ch)) {
      if (isupper(ch)) {
        // Leave upper register be, yet add SHIFT modifier.
        key.modifiers = ke_t::SHIFT;
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
            key.modifiers = ke_t::SHIFT;
            break;
          case '3':
            key.modifiers = ke_t::ALT;
            break;
          case '4':
            key.modifiers = ke_t::ALT | ke_t::SHIFT;
            break;
          case '5':
            key.modifiers = ke_t::CTRL;
            break;
          case '6':
            key.modifiers = ke_t::CTRL | ke_t::SHIFT;
            break;
          case '7':
            key.modifiers = ke_t::ALT | ke_t::CTRL;
            break;
          case '8':
            key.modifiers = ke_t::ALT | ke_t::SHIFT | ke_t::CTRL;
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
      key.modifiers |= ke_t::ALT;
      return key;
    }
  }

  // Try to convert from UTF-8
  LOG() << "Try to use utf8 cvt on " << data << " with size " << data.size();
  auto wstr = utf8_cvt_.from_bytes(data.begin(), data.end());
  if (utf8_cvt_.converted() == data.size() && wstr.size() == 1) {
    return ke_t{wstr[0]};
  }

  // Finally, give up.
  return {};
}

namespace {

int parse_decimal(std::string_view::iterator& i) {
  int num = 0;
  while (*i >= '0' && *i <= '9') {
    num = num * 10 + (*i++ - '0');
  }
  return num;
}

}  // namespace

std::optional<input::MouseEvent> Context::parse_mouse_event(
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

  input::MouseEvent mev{cx, cy};

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

namespace input {

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

bool KeyboardEvent::operator!=(wchar_t ch) const {
  using namespace base;
  return key != ch;
}

bool KeyboardEvent::operator==(wchar_t ch) const {
  using namespace base;
  return key == ch;
}

std::string MouseEvent::to_string() const {
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

}  // namespace input

namespace render {}  // namespace render

}  // namespace avada
