//
// Created by jeffset on 12/9/19.
//

#include "cursedui/rendering.hpp"

#include "base/util.hpp"
#include "cursesw.h"

#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <utility>

#ifdef border
#undef border
#endif

namespace std {
template <>
class hash<cursedui::render::RGB8Data> {
 public:
  size_t operator()(const cursedui::render::RGB8Data& color) const {
    return color.as_uint32();
  }
};
}  // namespace std

namespace cursedui::render {

struct BorderStyle {
  wchar_t top_left, top_right, bottom_right, bottom_left;
  wchar_t horizontal, vertical;
};

const BorderStyle BorderStyles::Single = {
    .horizontal = L'\u2500',
    .vertical = L'\u2502',

    .top_right = L'\u2510',
    .top_left = L'\u250C',
    .bottom_right = L'\u2518',
    .bottom_left = L'\u2514',
};

PIMPL_DEFINE(ColorPalette) {
  std::unordered_map<RGB8Data, base::ref_ptr<Color>> colors{};
  std::deque<uint8_t> free_indexes;

  // TODO: get rid of dynamic allocation here - it's redundant.
  std::array<base::ref_ptr<Color>, 8> system_colors{
      base::ref_ptr(new Color(COLOR_BLACK)), base::ref_ptr(new Color(COLOR_RED)),
      base::ref_ptr(new Color(COLOR_GREEN)), base::ref_ptr(new Color(COLOR_YELLOW)),
      base::ref_ptr(new Color(COLOR_BLUE)),  base::ref_ptr(new Color(COLOR_MAGENTA)),
      base::ref_ptr(new Color(COLOR_CYAN)),  base::ref_ptr(new Color(COLOR_WHITE)),
  };

  ColorPaletteImpl() {
    for (uint16_t i = system_colors.size(); i < COLORS; ++i) {
      free_indexes.push_back(static_cast<uint8_t>(i));
    }
  }

  // TODO: remove this gc to eager freeing of indexes.
  void gc() {
    std::unordered_map<RGB8Data, base::ref_ptr<Color>> gcollected;
    for (auto& [rgb, color] : colors) {
      if (color.ref_count() <= 1) {
        free_indexes.push_back(color->index_);
      } else {
        gcollected[rgb] = std::move(color);
      }
    }
    colors = std::move(gcollected);
  }

  uint8_t obtain_free_index() {
    if (free_indexes.empty()) {
      gc();
      if (free_indexes.empty())
        throw std::runtime_error("Palette is exhausted");
    }
    auto free_index = free_indexes.front();
    free_indexes.pop_front();
    return free_index;
  }
};

ColorPalette::ColorPalette() : PIMPL_INIT(ColorPalette) {}

ColorPalette::~ColorPalette() = default;

static void allocate_color(RGB8Data rgb, uint8_t index) {
  constexpr auto conv = [](uint8_t c) { return c * 1000 / 255; };
  ::init_color(index, conv(rgb.r), conv(rgb.g), conv(rgb.b));
}

base::ref_ptr<Color> ColorPalette::obtain_color(ColorDescr color_descr) {
  auto rgb_getter = [this](RGB8Data rgb) {
    auto& color = impl_->colors[rgb];
    if (color.ref_count() <= 1) {
      auto free_index = impl_->obtain_free_index();
      std::cerr << "Allocating color for index " << (int)free_index << '\n';
      allocate_color(rgb, free_index);
      color = base::ref_ptr(new Color(free_index));
    } else {
      std::cerr << "Using cached color for index " << (int)color->index_ << '\n';
    }
    return color;
  };
  auto system_getter = [this](Color::System system_color) {
    return impl_->system_colors[system_color];
  };
  return std::visit(base::overloaded{rgb_getter, system_getter}, color_descr);
}

PIMPL_DEFINE(Canvas) {
  uint8_t bg_color_index_ = COLOR_BLACK;
  uint8_t fg_color_index_ = COLOR_WHITE;
  short current_color_pair_;

  void ensure_color_pair() {
    if (current_color_pair_ < 0) {
      current_color_pair_ = ::alloc_pair(fg_color_index_, bg_color_index_);
      ::wattr_on(stdscr, COLOR_PAIR(current_color_pair_), nullptr);
    }
  }

  void set_background_color(uint8_t index) {
    if (bg_color_index_ != index) {
      bg_color_index_ = index;
      current_color_pair_ = -1;
    }
  }

  void set_foreground_color(uint8_t index) {
    if (fg_color_index_ != index) {
      fg_color_index_ = index;
      current_color_pair_ = -1;
    }
  }
};

Canvas::Canvas(void*) : PIMPL_INIT(Canvas) {}

void Canvas::start() {
  impl_->bg_color_index_ = COLOR_BLACK;
  impl_->fg_color_index_ = COLOR_WHITE;
  impl_->current_color_pair_ = -1;
  // WARNING: this will be wrong after more accurate repaints are implemented.
  ::wclear(stdscr);
}

Canvas::~Canvas() = default;

// static
void Canvas::init_rendering() {
  ::start_color();
  std::cerr << "COLORS=" << COLORS << "; COLOR_PAIRS=" << COLOR_PAIRS << std::endl;
}

Canvas& Canvas::operator<<(wchar_t ch) {
  impl_->ensure_color_pair();
  ::waddnwstr(stdscr, &ch, 1);
  return *this;
}

Canvas& Canvas::operator<<(const wchar_t* str) {
  impl_->ensure_color_pair();
  waddwstr(stdscr, str);
  return *this;
}

Canvas& Canvas::operator<<(const gfx::Point& pos) {
  auto r = ::wmove(stdscr, pos.y, pos.x);
  assert(r != ERR);
  return *this;
}

static void border(Canvas& canvas, const gfx::Rect& rect, const BorderStyle& style);

Canvas& Canvas::operator<<(const Box& box) {
  impl_->ensure_color_pair();
  border(*this, box.rect, *box.style);
  return *this;
}

ColorState2::ColorState2(Canvas* canvas)
    : canvas_(canvas), color_(canvas->impl_->bg_color_index_), valid_(true) {}

ColorState2::ColorState2() : valid_(false) {}

ColorState2::ColorState2(ColorState2&& state)
    : canvas_(state.canvas_), color_(state.color_), valid_(state.valid_) {
  state.valid_ = false;
}

ColorState2& ColorState2::operator=(ColorState2&& state) {
  if (this != &state) {
    this->~ColorState2();
    if (state.valid_) {
      canvas_ = state.canvas_;
      color_ = state.color_;
      valid_ = true;
      state.valid_ = false;
    } else {
      valid_ = false;
    }
  }
  return *this;
}

ColorState2::~ColorState2() {
  if (!valid_)
    return;
  canvas_->impl_->set_background_color(color_);
}

BgColorState Canvas::set_background_color(Color* color) {
  auto saved_state = BgColorState{this};
  impl_->set_background_color(color->index_);
  return saved_state;
}

void Canvas::set_foreground_color(Color* color) {
  impl_->set_foreground_color(color->index_);
}

namespace {

void hor_line(gfx::dim_t length, wchar_t ch) {
  cchar_t cc[]{cchar_t{}, {}};
  wchar_t c[] = {ch, 0};
  ::setcchar(&cc[0], c, 0, 0, nullptr);

  ::whline_set(stdscr, cc, length);
}

void ver_line(gfx::dim_t length, wchar_t ch) {
  cchar_t cc[]{cchar_t{}, {}};
  wchar_t c[] = {ch, 0};
  ::setcchar(&cc[0], c, 0, 0, nullptr);

  ::wvline_set(stdscr, cc, length);
}

}  // namespace

void fill(Canvas& canvas, wchar_t ch, const gfx::Rect& area) {
  auto y = area.top;
  for (; y <= area.bottom; ++y) {
    auto x = area.left;
    canvas << gfx::Point{x, y};
    for (; x <= area.right; ++x) {
      canvas << ch;
    }
  }
}

static void border(Canvas& canvas, const gfx::Rect& rect, const BorderStyle& style) {
  canvas << gfx::Point{rect.left, rect.top} << style.top_left;
  canvas << gfx::Point{rect.left, rect.bottom} << style.bottom_left;
  canvas << gfx::Point{rect.right, rect.top} << style.top_right;
  canvas << gfx::Point{rect.right, rect.bottom} << style.bottom_right;

  auto size = rect.size();
  if (size.width > 2) {
    canvas << gfx::Point{rect.left + 1, rect.top};
    hor_line(size.width - 2, style.horizontal);
    canvas << gfx::Point{rect.left + 1, rect.bottom};
    hor_line(size.width - 2, style.horizontal);
  }
  if (size.height > 2) {
    canvas << gfx::Point{rect.left, rect.top + 1};
    ver_line(size.height - 2, style.vertical);
    canvas << gfx::Point{rect.right, rect.top + 1};
    ver_line(size.height - 2, style.vertical);
  }
}

}  // namespace cursedui::render
