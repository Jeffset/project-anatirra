//
// Created by jeffset on 12/9/19.
//

#include "cursedui/context.hpp"

#include "base/util.hpp"
#include "cursedui/input.hpp"
#include "cursedui/rendering.hpp"
#include "cursedui/view.hpp"
#include "cursedui/view_tree_host.hpp"
#include "cursesw.h"

#include <clocale>
#include <csignal>
#include <iostream>
#include <optional>

#define KEY_ALT(x) (KEY_F(64 - 26) + (x - 'A'))

namespace {

volatile std::sig_atomic_t g_siginted = false;

void sigint_handler(int) {
  g_siginted = true;
}

}  // namespace

namespace cursedui {

PIMPL_DEFINE(Context) {
  uint32_t mouse_bstate;

  input::InputEvent to_input_event(int code) {
    using namespace input;
    switch (code) {
      case KEY_MOUSE: {
        MEVENT mouse_event;
        ::getmouse(&mouse_event);
        gfx::Point location = {mouse_event.x, mouse_event.y};
        if (mouse_event.bstate & BUTTON4_PRESSED) {
          return ScrollEvent{location, ScrollDirection::UP};
        } else if (mouse_event.bstate & BUTTON5_PRESSED) {
          return ScrollEvent{location, ScrollDirection::DOWN};
        }

        MouseEvent ev;
        ev.location = location;
        auto state = mouse_event.bstate;
        if (!(mouse_bstate & BUTTON1_PRESSED) && (state & BUTTON1_PRESSED)) {
          ev.event_code = MouseEventCode::LEFT_DOWN;
        } else if ((mouse_bstate & BUTTON1_PRESSED) && !(state & BUTTON1_PRESSED)) {
          ev.event_code = MouseEventCode::LEFT_UP;
        } else if (!(mouse_bstate & BUTTON2_PRESSED) && (state & BUTTON2_PRESSED)) {
          ev.event_code = MouseEventCode::RIGHT_DOWN;
        } else if ((mouse_bstate & BUTTON2_PRESSED) && !(state & BUTTON2_PRESSED)) {
          ev.event_code = MouseEventCode::RIGHT_UP;
        } else {
          assert(false);
        }
        mouse_bstate = state;
        return ev;
      }
      case '\t':
        return KeyEvent{Key::TAB, '\t'};
      case ' ':
        return KeyEvent{Key::SPACE, ' '};
      case '\n':
      case 13:
      case KEY_ENTER:
        return KeyEvent{Key::ENTER, '\n'};
      case KEY_IC:
        return KeyEvent{Key::INSERT, {}};
      case KEY_DC:
        return KeyEvent{Key::DELETE, {}};
      case 127:
      case KEY_BACKSPACE:
        return KeyEvent{Key::BACKSPACE, {}};
      case KEY_PPAGE:
        return KeyEvent{Key::PAGE_UP, {}};
      case KEY_NPAGE:
        return KeyEvent{Key::PAGE_DOWN, {}};
      case KEY_HOME:
        return KeyEvent{Key::HOME, {}};
      case KEY_END:
        return KeyEvent{Key::END, {}};
      case KEY_F(1):
        return KeyEvent{Key::F1, {}};
      case KEY_F(2):
        return KeyEvent{Key::F2, {}};
      case KEY_F(3):
        return KeyEvent{Key::F3, {}};
      case KEY_F(4):
        return KeyEvent{Key::F4, {}};
      case KEY_F(5):
        return KeyEvent{Key::F5, {}};
      case KEY_F(6):
        return KeyEvent{Key::F6, {}};
      case KEY_F(7):
        return KeyEvent{Key::F7, {}};
      case KEY_F(8):
        return KeyEvent{Key::F8, {}};
      case KEY_F(9):
        return KeyEvent{Key::F9, {}};
      case KEY_F(10):
        return KeyEvent{Key::F10, {}};
      case KEY_F(11):
        return KeyEvent{Key::F11, {}};
      case KEY_F(12):
        return KeyEvent{Key::F12, {}};
      default:
        return KeyEvent{Key::OTHER, code};
    }
  }
};

Context::Context() : PIMPL_INIT(Context) {
  std::setlocale(LC_ALL, "");

  ::initscr();
  ::noecho();
  ::cbreak();
  ::nonl();
  ::intrflush(stdscr, FALSE);

  ::curs_set(0);
  ::mouseinterval(0);

  ::keypad(stdscr, true);

  //::printf("\033[?1003h\n");  // Makes the terminal report mouse movement events

  ::define_key("\033[H", KEY_HOME);
  ::define_key("\033[F", KEY_END);
  ::define_key("\033[7~", KEY_HOME);
  ::define_key("\033[8~", KEY_END);
  ::define_key("\033OP", KEY_F(1));
  ::define_key("\033OQ", KEY_F(2));
  ::define_key("\033OR", KEY_F(3));
  ::define_key("\033OS", KEY_F(4));
  ::define_key("\033[11~", KEY_F(1));
  ::define_key("\033[12~", KEY_F(2));
  ::define_key("\033[13~", KEY_F(3));
  ::define_key("\033[14~", KEY_F(4));
  ::define_key("\033[17;2~", KEY_F(18));
  char sequence[3] = "\033a";
  for (char c = 'a'; c <= 'z'; c++) {
    sequence[1] = c;
    ::define_key(sequence, KEY_ALT('A' + (c - 'a')));
  }

  ::mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);

  std::signal(SIGINT, sigint_handler);

  render::Canvas::init_rendering();
}

Context::~Context() {
  //::printf("\033[?1003l\n");  // Disable mouse movement events, as l = low
  ::endwin();
}

void Context::run(view::ViewTreeHost* view_tree_host) {
  const auto input_dispatcher =
      base::overloaded{[view_tree_host](const input::KeyEvent& key_event) {
                         view_tree_host->dispatch_key_event(key_event);
                       },
                       [view_tree_host](const input::MouseEvent& event) {
                         view_tree_host->dispatch_mouse_event(event);
                       },
                       [view_tree_host](const input::ScrollEvent& event) {
                         view_tree_host->dispatch_scroll_event(event);
                       }};

  render::Canvas canvas{nullptr};
  render::ColorPalette palette{};
  try {
    int ch;
    view_tree_host->render(canvas, palette);
    do {
      ch = ::wgetch(stdscr);
      switch (ch) {
        case KEY_RESIZE:
          view_tree_host->render(canvas, palette);
          ::refresh();
          break;
        default: {
          auto event = impl_->to_input_event(ch);
          std::visit(input_dispatcher, event);
          break;
        }
      }
    } while (ch != 0 && !g_siginted);
  } catch (view::view_exception& e) {
    std::cerr << "view_exception: " << e.what() << '\n';
    return;
  } catch (render::render_exception& e) {
    std::cerr << "render_exception: " << e.what() << '\n';
    return;
  }
}

gfx::Size Context::screen_size() const {
  gfx::Size size{};
  getmaxyx(stdscr, size.height, size.width);
  return size;
}

}  // namespace cursedui
