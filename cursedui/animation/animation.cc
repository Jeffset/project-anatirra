// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/animation/animation.hpp"

namespace cursedui::animation {

void Animation::set_attached(bool attached) noexcept {
  attached_ = attached;
}

Animation::Animation() noexcept : attached_(false), finished_(false) {}

Animation::~Animation() noexcept = default;

}  // namespace cursedui::animation
