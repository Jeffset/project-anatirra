// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_REGION
#define ANATIRRA_CURSEDUI_REGION

#include "cursedui_config.hpp"

#include "cursedui/dim.hpp"

#include <list>

namespace cursedui::paint {

class CURSEDUI_PUBLIC Region {
 public:
  Region() noexcept = default;
  explicit Region(const gfx::Rect& rect) noexcept;

  void add(const gfx::Rect& rect) noexcept;
  void add(const Region& region) noexcept;
  void add(Region&& region) noexcept;

  Region clip(const gfx::Rect& rect) const noexcept;
  Region clip(const Region& region) const noexcept;
  bool clip(const gfx::Point& point) const noexcept;

  bool empty() const noexcept;

  auto begin() const noexcept { return rects_.cbegin(); }
  auto end() const noexcept { return rects_.cend(); }

 private:
  // TODO: Maybe use quadro-trees?
  std::list<gfx::Rect> rects_;
};

}  // namespace cursedui::paint

#endif  // ANATIRRA_CURSEDUI_REGION
