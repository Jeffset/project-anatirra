// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_ANIMATION_ANIMATION
#define ANATIRRA_CURSEDUI_ANIMATION_ANIMATION

#include "base/macro.hpp"
#include "base/weak_ref.hpp"

#include "cursedui_config.hpp"

#include <chrono>

namespace cursedui::animation {
using namespace std::chrono_literals;

class CURSEDUI_PUBLIC Animation : public base::WeakReferenced {
 public:
  using clock_t = std::chrono::steady_clock;
  using duration_t = clock_t::duration;
  using time_point_t = clock_t::time_point;

  static constexpr duration_t interval_v = 50ms;

 public:
  bool is_finished() const noexcept { return finished_; }

 private:
  friend class AnimationHost;

  void set_attached(bool attached) noexcept;
  bool is_attached() const noexcept { return attached_; }
  bool attached_;

 protected:
  Animation() noexcept;
  ~Animation() noexcept;

  virtual void on_frame() = 0;

  bool finished_;
};

}  // namespace cursedui::animation

#endif  // ANATIRRA_CURSEDUI_ANIMATION_ANIMATION
