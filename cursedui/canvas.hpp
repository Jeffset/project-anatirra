// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_CANVAS
#define ANATIRRA_CURSEDUI_CANVAS

#include "avada/color.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/region.hpp"

#include <array>
#include <stack>
#include <string_view>
#include <variant>

namespace avada::render {
class Buffer;
}

namespace cursedui::paint {

enum class BlendMode : uint8_t { BLIT = 0, BLEND = 1 };

struct Pen {
  avada::render::Color fg_color;
  BlendMode fg_blend_mode;

  avada::render::Color bg_color;
  BlendMode bg_blend_mode;

  uint8_t render_attributes;
  BlendMode render_attributes_blend_mode;

  Pen(avada::render::Color fg_color,
      avada::render::Color bg_color,
      BlendMode fg_blend_mode = BlendMode::BLEND,
      BlendMode bg_blend_mode = BlendMode::BLEND,
      uint8_t render_attributes = 0x0,
      BlendMode render_attributes_blend_mode = BlendMode::BLIT)
      : fg_color(fg_color),
        fg_blend_mode(fg_blend_mode),
        bg_color(bg_color),
        bg_blend_mode(bg_blend_mode),
        render_attributes(render_attributes),
        render_attributes_blend_mode(render_attributes_blend_mode) {}
};

enum class Direction : uint8_t { HORIZONTAL, VERTICAL };

class Canvas {
 public:
  explicit Canvas(avada::render::Buffer& buffer) noexcept;

  class ScopedClipHandle {
   public:
    ~ScopedClipHandle();
    DISABLE_COPY_MOVE(ScopedClipHandle);

    operator bool() const { return culled_; }

   private:
    friend class Canvas;

    ScopedClipHandle(Canvas& canvas, bool culled) : self_(canvas), culled_(culled) {}

    Canvas& self_;
    const bool culled_;
  };

  // Basic painting API:
  template <typename Char>
  void draw(std::basic_string_view<Char> str, gfx::Point start, const Pen& pen) noexcept;

  template <typename Char>
  void draw(Char c, gfx::Point position, const Pen& pen) noexcept;

  template <typename Char>
  void draw_line(Char c,
                 gfx::Point start,
                 Direction direction,
                 gfx::dim_t count,
                 const Pen& pen) noexcept;

  template <typename Char>
  void fill(Char c, gfx::Rect rect, const Pen& pen) noexcept;

  void draw(const avada::render::Buffer& buffer,
            gfx::Point target_position,
            BlendMode blend_mode) noexcept;
  void draw(const avada::render::Buffer& buffer,
            gfx::Rect source_rect,
            gfx::Point target_position,
            BlendMode blend_mode) noexcept;

  ScopedClipHandle push_clip(const gfx::Rect& rect) noexcept;
  ScopedClipHandle push_clip(const paint::Region& region) noexcept;

  const avada::render::Buffer& buffer() const noexcept { return buffer_; }

 private:
  void pop_clip() noexcept;

 private:
  paint::Region apply_clip(const gfx::Rect& rect) const noexcept;
  gfx::Rect clip_to_buffer(const gfx::Rect& rect) const noexcept;

 private:
  std::stack<paint::Region> clip_stack_;
  avada::render::Buffer& buffer_;
};

}  // namespace cursedui::paint

#endif  // ANATIRRA_CURSEDUI_CANVAS
