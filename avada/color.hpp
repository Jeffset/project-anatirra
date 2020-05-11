// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_AVADA_COLOR
#define ANATIRRA_AVADA_COLOR

#include <cstdint>
#include <variant>

namespace avada::render {

union ColorRGB {
  uint32_t data_;
  alignas(uint32_t) uint8_t rgba_[4];

  inline ColorRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
      : rgba_{red, green, blue, alpha} {}

  inline explicit ColorRGB(uint32_t rgb) : data_{rgb} {}
  inline ColorRGB() : data_(0) {}

  inline uint8_t red() const noexcept { return rgba_[0]; }
  inline uint8_t green() const noexcept { return rgba_[1]; }
  inline uint8_t blue() const noexcept { return rgba_[2]; }
  inline uint8_t alpha() const noexcept { return rgba_[3]; }

  inline uint8_t& red() noexcept { return rgba_[0]; }
  inline uint8_t& green() noexcept { return rgba_[1]; }
  inline uint8_t& blue() noexcept { return rgba_[2]; }
  inline uint8_t& alpha() noexcept { return rgba_[3]; }

  inline bool operator==(const ColorRGB& rhs) const noexcept {
    return data_ == rhs.data_;
  }
  inline bool operator!=(const ColorRGB& rhs) const noexcept {
    return data_ != rhs.data_;
  }
};

struct Colors {
  Colors() = delete;

  static const ColorRGB TRANSPARENT;
  static const ColorRGB BLACK;
  static const ColorRGB WHITE;
};

struct RenderAttributes {
  RenderAttributes() = delete;

  static constexpr uint8_t BOLD = 1 << 0;
  static constexpr uint8_t ITALIC = 1 << 1;
  static constexpr uint8_t UNDERLINE = 1 << 2;
};

enum class SystemColor : uint8_t {
  BLACK = 0,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  DEFAULT,
};

using Color = std::variant<ColorRGB, SystemColor>;

Color alpha_blend(const Color& source, const Color& destination) noexcept;

}  // namespace avada::render

#endif  // ANATIRRA_AVADA_COLOR
