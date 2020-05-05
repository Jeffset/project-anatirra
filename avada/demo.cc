// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/avada.hpp"
#include "base/debug.hpp"
#include "base/util.hpp"

class EventHandler {
 public:
  EventHandler(avada::Context& context) : context_(context), color_{128, 0, 255} {}

  void operator()(avada::input::ResizeEvent resize) {
    LOG() << "ResizeEvent: " << resize.rows << " x " << resize.columns;
    context_.render_buffer().clear();
    render_scene(context_.render_buffer(), '$');
    context_.swap_buffers();
  }

  void operator()(avada::input::ServiceEvent) {}

  void operator()(avada::input::KeyboardEvent ev) {
    using namespace avada::input;
    if (ev == KeyboardEvent{L'q', KeyboardEvent::CTRL}) {
      should_exit_ = true;
    } else if (ev == L'p') {
      render_scene(context_.render_buffer(), '#');
      context_.swap_buffers();
    } else if (ev == L'o') {
      render_scene(context_.render_buffer(), '@');
      context_.swap_buffers();
    }
    LOG() << "KeyboardEvent: " << ev.to_string();
  }

  void operator()(avada::input::MouseEvent ev) {
    using namespace avada::input;
    std::visit(base::overloaded{[this](MouseEvent::ButtonEvent be) {
                                  if (be.code == MouseEvent::Button::LEFT) {
                                    is_drawing_ = be.state == MouseEvent::State::PRESSED;
                                  }
                                },
                                [this, &ev](std::monostate) {
                                  if (is_drawing_)
                                    draw(context_.render_buffer(), ev.x, ev.y);
                                },
                                [this](MouseEvent::Scroll scroll) {
                                  if (scroll == MouseEvent::Scroll::UP) {
                                    color_.g = std::min(color_.g + 1, 255);
                                  } else {
                                    color_.g = std::max(color_.g - 1, 0);
                                  }
                                }},
               ev.data);
    LOG() << "MouseEvent: " << ev.to_string();
  }

  bool should_exit() const noexcept { return should_exit_; }

 private:
  void draw(avada::render::Buffer& buffer, int x, int y) {
    using namespace avada::render;
    auto& cell = buffer(y - 1, x - 1);
    cell.set_data('0');
    cell.set_fg_color(color_);
    cell.set_attributes(Buffer::ATTRIB_BOLD);
    context_.swap_buffers();
  }

  void render_scene(avada::render::Buffer& buffer, char c) {
    int w = buffer.columns() / 2;
    int h = buffer.rows() / 2;

    int i1 = (buffer.rows() - h) / 2;
    int j1 = (buffer.columns() - w) / 2;
    int i2 = i1 + h;
    int j2 = j1 + w;

    LOG() << "w: " << w << "; h: " << h << "; i1: " << i1 << "; j1: " << j1;

    for (int i = i1; i < i2; ++i) {
      for (int j = j1; j < j2; ++j) {
        buffer(i, j).set_data(c);
      }
    }
  }

 private:
  avada::Context& context_;
  avada::render::Color color_;
  bool should_exit_;
  bool is_drawing_;
};

int main() {
  try {
    LOG() << "Start";
    avada::Context context;

    EventHandler hander{context};
    while (!hander.should_exit()) {
      std::visit(hander, context.poll_event());
    }

    LOG() << "Nice exit";
    return 0;

  } catch (base::system_exception& e) {
    LOG() << "System exception: " << e.what();
    return 1;
  } catch (std::exception& e) {
    LOG() << "Unexpected exception: " << e.what();
    return 2;
  }
}
