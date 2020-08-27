// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/animation/animations.hpp"

#include "avada/color.hpp"
#include "base/debug/debug.hpp"
#include "cursedui/view.hpp"
#include "cursedui/view_tree_host.hpp"

namespace cursedui::animation {

namespace {

using avada::render::ColorRGB;

template <class T>
T lerp(const T& from, const T& to, double progress) {
  return from + (to - from) * progress;
}

template <>
ColorRGB lerp<ColorRGB>(const ColorRGB& from, const ColorRGB& to, double progress) {
  return {
      lerp(from.red(), to.red(), progress),
      lerp(from.green(), to.green(), progress),
      lerp(from.blue(), to.blue(), progress),
      lerp(from.alpha(), to.alpha(), progress),
  };
}

}  // namespace

void start_view_animation(view::View* view, Animation* animation) {
  view->tree_host().get()->animation_host().start(animation);
}

template <class T>
void AnimationValue<T>::set(T value) noexcept {
  if (value_ == value)
    return;
  value_ = value;
  if (influence_.has(REPAINT)) {
    mark_needs_repaint();
  }
  if (influence_.has(RELAYOUT_WIDTH)) {
    mark_needs_layout(view::NeedsLayout::WIDTH);
  }
  if (influence_.has(RELAYOUT_HEIGHT)) {
    mark_needs_layout(view::NeedsLayout::HEIGHT);
  }
}

template <class T>
ValueAnimation<T>::ValueAnimation(AnimationValue<T>& value,
                                  T from,
                                  T to,
                                  Animation::duration_t duration) noexcept
    : value_(value), from_(from), to_(to), duration_(duration) {
  if (from_ == to_) {
    finished_ = true;
  }
}

template <class T>
ValueAnimation<T>::ValueAnimation(AnimationValue<T>& value,
                                  T to,
                                  Animation::duration_t duration) noexcept
    : ValueAnimation(value, value, to, duration) {}

template <class T>
void ValueAnimation<T>::on_frame() {
  if (!start_time_.has_value()) {
    start_time_ = clock_t::now();
    value_.set(from_);
    return;
  }

  const auto passed = clock_t::now() - start_time_.value();
  const auto progress = static_cast<double>(passed.count()) / duration_.count();
  LOG() << "progress " << progress;
  if (progress > 1.0) {
    value_.set(to_);
    finished_ = true;
    return;
  }
  value_.set(lerp(from_, to_, progress));
}

template class CURSEDUI_PUBLIC ValueAnimation<ColorRGB>;
template class CURSEDUI_PUBLIC ValueAnimation<gfx::dim_t>;
template class CURSEDUI_PUBLIC ValueAnimation<double>;

}  // namespace cursedui::animation
