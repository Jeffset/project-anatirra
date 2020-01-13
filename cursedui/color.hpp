#ifndef COLOR_HPP
#define COLOR_HPP

#include "base/ref_ptr.hpp"

#include <cstdint>
#include <memory>
#include <variant>

namespace cursedui::render {

class Canvas;

struct RGB8Data {
  uint8_t r, g, b;

  uint32_t as_uint32() const { return b | (g << 8) | (r << 16); }

  bool operator==(const RGB8Data& other) const {
    return as_uint32() == other.as_uint32();
  }
  bool operator!=(const RGB8Data& other) const {
    return as_uint32() != other.as_uint32();
  }
};

enum class SystemColor : uint8_t {
  BLACK = 0,
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  MAGENTA = 5,
  CYAN = 6,
  WHITE = 7
};

using color_id_t = long;
using ColorDescr = std::variant<RGB8Data, SystemColor>;

class Color : public base::RefCounted {
 private:
  Color(uint8_t index, ColorDescr descr) : index_(index), descr_(descr) {}
  explicit Color(SystemColor system_color)
      : index_(static_cast<uint8_t>(system_color)), descr_(system_color) {}
  const uint8_t index_;
  ColorDescr descr_;

  friend class Canvas;
  friend class ColorPalette;
};

class ColorState2 {
 private:
  explicit ColorState2(Canvas* canvas);
  friend class Canvas;

 public:
  ColorState2();
  ColorState2(ColorState2&&);
  ColorState2& operator=(ColorState2&&);
  ~ColorState2();

  DISABLE_COPY_AND_ASSIGN(ColorState2);

 private:
  Canvas* canvas_;
  uint8_t color_;
  bool valid_;
};

using BgColorState = ColorState2;

}  // namespace cursedui::render

#endif  // COLOR_HPP
