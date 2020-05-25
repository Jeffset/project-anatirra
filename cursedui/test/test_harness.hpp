// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_CURSEDUI_TEST_TEST_HARNESS
#define ANATIRRA_CURSEDUI_TEST_TEST_HARNESS

#include "base/util.hpp"
#include "cursedui/view.hpp"

namespace cursedui::view::test {

class ViewForTest : public View {
 public:
  explicit ViewForTest(gfx::Size preferred_size) : preferred_size_(preferred_size) {}

 private:
  constexpr static auto make_measurer(gfx::dim_t preferred) {
    return base::overloaded{
        [](MeasureExactly spec) { return spec.dim; },
        [preferred](MeasureAtMost at_most) { return std::min(at_most.dim, preferred); },
        [preferred](MeasureUnlimited) { return preferred; }};
  }

  gfx::Size preferred_size_;

 protected:
  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec, bool) override {
    return {std::visit(make_measurer(preferred_size_.width), width_spec),
            std::visit(make_measurer(preferred_size_.height), height_spec)};
  }
};

inline auto make_view(gfx::Size preferred_size = {10, 10}) {
  return base::make_ref_ptr<test::ViewForTest>(preferred_size);
}

#define EXPECT_BOUNDS(view, bounds) EXPECT_EQ(view->outer_bounds(), (bounds))

}  // namespace cursedui::view::test

#endif  // ANATIRRA_CURSEDUI_TEST_TEST_HARNESS
