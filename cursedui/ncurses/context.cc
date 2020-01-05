//
// Created by jeffset on 12/9/19.
//

#include "cursedui/context.hpp"

#include "cursedui/rendering.hpp"

#include <clocale>
#include <cursesw.h>

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

  ::start_color();
  ::init_extended_color(228, 700, 1000, 300);
  ::init_pair(1, 228, COLOR_BLACK);
  ::attron(COLOR_PAIR(1));
}

Context::~Context() {
  ::endwin();
}

void Context::run(Delegate* delegate) {
  render::Canvas canvas{nullptr};
  int ch;
  do {
    delegate->render(canvas);
    ::refresh();
    ch = ::wgetch(stdscr);
  } while (ch != 0);
}

gfx::Size Context::screen_size() const {
  gfx::Size size{};
  getmaxyx(stdscr, size.height, size.width);
  return size;
}

}  // namespace cursedui
