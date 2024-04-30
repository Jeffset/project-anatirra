// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/frame_layout.hpp"

#include "base/debug/debug.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/test/test_harness.hpp"

#include "gtest/gtest.h"

using namespace cursedui;
using namespace cursedui::gfx;
using namespace cursedui::view;

TEST(FrameLayoutTest, RootChildless) {
  auto fl = base::make_ref_ptr<FrameLayout>();

  fl->layout_as_root({0, 0, 100, 100});

  EXPECT_EQ(fl->outer_bounds(), (Rect{0, 0, 100, 100}));
}

TEST(FrameLayoutTest, RootWithChild) {
  auto fl = base::make_ref_ptr<FrameLayout>();
  fl->add_child(test::make_view());

  fl->layout_as_root({0, 0, 100, 100});

  EXPECT_EQ(fl->outer_bounds(), (Rect{0, 0, 100, 100}));
}

TEST(FrameLayoutTest, RootWithChild_Oversize) {
  auto fl = base::make_ref_ptr<FrameLayout>();
  auto child = test::make_view();
  add_child(fl, child, LayoutExactly{240}, LayoutExactly{200});

  fl->layout_as_root({0, 0, 100, 100});

  EXPECT_EQ(fl->outer_bounds(), (Rect{0, 0, 100, 100}));
  EXPECT_BOUNDS(child, rect_from({-70, -50}, {240, 200}));
}

TEST(FrameLayoutTest, WithChildCenter_WrapContent) {
  auto root = base::make_ref_ptr<FrameLayout>();
  auto fl = base::make_ref_ptr<FrameLayout>();
  add_child(root, fl, LayoutWrapContent{}, LayoutWrapContent{});

  auto v = test::make_view();
  v->border().set_style(BorderDrawable::Style::SINGLE);
  add_child(fl, v, LayoutWrapContent{}, LayoutWrapContent{});

  root->layout_as_root(rect_from({0, 0}, {100, 100}));

  EXPECT_EQ(fl->outer_bounds(), rect_from({44, 44}, {12, 12}));
  EXPECT_EQ(v->outer_bounds(), rect_from({44, 44}, {12, 12}));
}

TEST(FrameLayoutTest, WithChildCenter_MatchParent) {
  auto root = base::make_ref_ptr<FrameLayout>();
  auto fl = base::make_ref_ptr<FrameLayout>();
  add_child(root, fl, LayoutMatchParent{}, LayoutMatchParent{});

  auto v = test::make_view();
  v->border().set_style(BorderDrawable::Style::NO_BORDER);
  add_child(fl, v, LayoutWrapContent{}, LayoutWrapContent{});

  root->layout_as_root(rect_from({0, 0}, {100, 100}));

  EXPECT_EQ(fl->outer_bounds(), rect_from({0, 0}, {100, 100}));
  EXPECT_EQ(v->outer_bounds(), rect_from({45, 45}, {10, 10}));
}

TEST(FrameLayoutTest, MultipleGravity) {
  using namespace base::operators;

  auto root = base::make_ref_ptr<FrameLayout>();
  auto fl = base::make_ref_ptr<FrameLayout>();
  add_child(root, fl, LayoutMatchParent{}, LayoutMatchParent{});

  auto left = test::make_view();
  auto right = test::make_view();
  auto top = test::make_view();
  auto bottom = test::make_view();

  auto left_top = test::make_view();
  auto right_top = test::make_view();
  auto left_bottom = test::make_view();
  auto right_bottom = test::make_view();

  auto top_bar = test::make_view();
  auto bottom_bar = test::make_view();
  auto left_column = test::make_view();
  auto right_column = test::make_view();

  constexpr auto wc = LayoutWrapContent{};
  add_child(fl, left, wc, wc, Gravity::LEFT);
  add_child(fl, right, wc, wc, Gravity::RIGHT);
  add_child(fl, top, wc, wc, Gravity::TOP);
  add_child(fl, bottom, wc, wc, Gravity::BOTTOM);

  add_child(fl, left_top, wc, wc, Gravity::TOP | Gravity::LEFT);
  add_child(fl, right_top, wc, wc, Gravity::TOP | Gravity::RIGHT);
  add_child(fl, left_bottom, wc, wc, Gravity::BOTTOM | Gravity::LEFT);
  add_child(fl, right_bottom, wc, wc, Gravity::BOTTOM | Gravity::RIGHT);

  add_child(fl, top_bar, wc, wc, Gravity::LEFT | Gravity::RIGHT | Gravity::TOP);
  add_child(fl, bottom_bar, wc, wc, Gravity::LEFT | Gravity::RIGHT | Gravity::BOTTOM);
  add_child(fl, left_column, wc, wc, Gravity::LEFT | Gravity::TOP | Gravity::BOTTOM);
  add_child(fl, right_column, wc, wc, Gravity::RIGHT | Gravity::TOP | Gravity::BOTTOM);

  root->layout_as_root(rect_from({0, 0}, {100, 100}));

  EXPECT_BOUNDS(left, rect_from({0, 44}, {12, 12}));
  EXPECT_BOUNDS(left_column, rect_from({0, 44}, {12, 12}));

  EXPECT_BOUNDS(right, rect_from({88, 44}, {12, 12}));
  EXPECT_BOUNDS(right_column, rect_from({88, 44}, {12, 12}));

  EXPECT_BOUNDS(top, rect_from({44, 0}, {12, 12}));
  EXPECT_BOUNDS(top_bar, rect_from({44, 0}, {12, 12}));

  EXPECT_BOUNDS(bottom, rect_from({44, 88}, {12, 12}));
  EXPECT_BOUNDS(bottom_bar, rect_from({44, 88}, {12, 12}));

  EXPECT_BOUNDS(left_top, rect_from({0, 0}, {12, 12}));
  EXPECT_BOUNDS(right_top, rect_from({88, 0}, {12, 12}));
  EXPECT_BOUNDS(left_bottom, rect_from({0, 88}, {12, 12}));
  EXPECT_BOUNDS(right_bottom, rect_from({88, 88}, {12, 12}));
}

TEST(FrameLayoutTest, CrossMatchParent_WithWrapContent) {
  auto root = base::make_ref_ptr<FrameLayout>();
  auto fl = base::make_ref_ptr<FrameLayout>();

  constexpr auto wc = LayoutWrapContent{};
  constexpr auto mp = LayoutMatchParent{};

  add_child(root, fl, wc, wc);

  auto c1 = test::make_view({10, 20});
  c1->border().set_style(BorderDrawable::Style::NO_BORDER);
  add_child(fl, c1, wc, mp);
  auto c2 = test::make_view({30, 15});
  c2->border().set_style(BorderDrawable::Style::NO_BORDER);
  add_child(fl, c2, mp, wc);

  root->layout_as_root(rect_from({0, 0}, {100, 100}));

  EXPECT_BOUNDS(fl, rect_from({35, 40}, {30, 20}));

  EXPECT_BOUNDS(c1, rect_from({45, 40}, {10, 20}));
  EXPECT_BOUNDS(c2, rect_from({35, 43}, {30, 15}));
}

TEST(FrameLayoutTest, CrossMatchParentComplex) {
  auto root = base::make_ref_ptr<FrameLayout>();
  auto fl = base::make_ref_ptr<FrameLayout>();

  constexpr auto wc = LayoutWrapContent{};
  constexpr auto mp = LayoutMatchParent{};
  constexpr auto ex = LayoutExactly{50};

  add_child(root, fl, wc, wc);

  auto c1 = test::make_view();
  add_child(fl, c1, wc, mp);
  auto c2 = test::make_view();
  add_child(fl, c2, mp, wc);

  auto c3 = test::make_view();
  add_child(fl, c3, ex, mp);

  auto c4 = test::make_view();
  add_child(fl, c4, mp, ex);

  root->layout_as_root(rect_from({0, 0}, {100, 100}));

  EXPECT_BOUNDS(fl, rect_from({25, 25}, {50, 50}));

  EXPECT_BOUNDS(c1, rect_from({44, 25}, {12, 50}));
  EXPECT_BOUNDS(c2, rect_from({25, 44}, {50, 12}));

  EXPECT_BOUNDS(c3, rect_from({25, 25}, {50, 50}));
  EXPECT_BOUNDS(c4, rect_from({25, 25}, {50, 50}));
}
