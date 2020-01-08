//
// Created by jeffset on 12/9/19.
//

#include "cursedui/context.hpp"

#include "cursedui/rendering.hpp"
#include "cursedui/view.hpp"
#include "cursesw.h"

#include <clocale>
#include <iostream>

namespace cursedui {

struct Context::ContextImpl {};

Context::Context() {
  std::setlocale(LC_ALL, "");

  ::initscr();
  ::noecho();
  ::cbreak();
  ::nonl();
  ::intrflush(stdscr, FALSE);

  ::curs_set(0);
  ::mouseinterval(0);

  ::keypad(stdscr, true);

  ::mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);

  render::Canvas::init_rendering();
}

Context::~Context() {
  ::endwin();
}

void Context::run(Delegate* delegate) {
  render::Canvas canvas{nullptr};
  render::ColorPalette palette{};
  int ch;
  do {
    try {
      delegate->render(canvas, palette);
    } catch (view::view_exception& e) {
      std::cerr << "view_exception: " << e.what() << '\n';
      return;
    } catch (render::render_exception& e) {
      std::cerr << "render_exception: " << e.what() << '\n';
      return;
    }
    ::refresh();
    ch = ::wgetch(stdscr);
    if (ch == 'q') {
      return;
    } else if (ch == KEY_MOUSE) {
      MEVENT mouse_event;
      if (::getmouse(&mouse_event) == OK) {
        std::cerr << "MOUSE = x: " << mouse_event.x << " y: " << mouse_event.y
                  << "z: " << mouse_event.z << "; id: " << mouse_event.id
                  << "bstate: " << mouse_event.bstate << std::endl;
      }
    }
  } while (ch != 0);
}

gfx::Size Context::screen_size() const {
  gfx::Size size{};
  getmaxyx(stdscr, size.height, size.width);
  return size;
}

}  // namespace cursedui
