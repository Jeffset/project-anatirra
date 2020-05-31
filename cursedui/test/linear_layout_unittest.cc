// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/views/linear_layout.hpp"

#include "base/debug/debug.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/test/test_harness.hpp"
#include "cursedui/views/frame_layout.hpp"
#include "third-party/googletest/gtest.hpp"

using namespace cursedui;
using namespace cursedui::gfx;
using namespace cursedui::view;

namespace {

constexpr auto wc = LayoutWrapContent{};
constexpr auto mp = LayoutMatchParent{};
constexpr auto ex(gfx::dim_t dim) {
  return LayoutExactly{dim};
}

}  // namespace

class LinearLayoutTestBase {
 protected:
  base::ref_ptr<FrameLayout> root_ = base::make_ref_ptr<FrameLayout>();
  base::ref_ptr<LinearLayout> ll_ = base::make_ref_ptr<LinearLayout>();
  void layout() { root_->layout_as_root(rect_from({}, {100, 100})); }
  void horizontal() { ll_->set_orientation(LinearLayout::HORIZONTAL); }
  void vertical() { ll_->set_orientation(LinearLayout::VERTICAL); }
  template <class... Args>
  auto add_child(Args&&... args) {
    auto child = test::make_view();
    view::add_child(ll_, child, std::forward<Args>(args)...);
    return child;
  }

  virtual void add_to_root() { ::add_child(root_, ll_, mp, mp); }
};

class LinearLayoutTest : public LinearLayoutTestBase, public ::testing::Test {
  void SetUp() override { add_to_root(); }
};
class LinearLayoutTestWC : public LinearLayoutTestBase, public ::testing::Test {
 protected:
  void add_to_root() override {
    using namespace base::operators;
    ::add_child(root_, ll_, wc, wc, Gravity::LEFT | Gravity::TOP);
  }
  void SetUp() override { add_to_root(); }
};

class LinearLayoutTestWithOrientation
    : public LinearLayoutTestBase,
      public ::testing::TestWithParam<LinearLayout::Orientation> {
 protected:
  void SetUp() override {
    add_to_root();
    ll_->set_orientation(GetParam());
  }
};

TEST_F(LinearLayoutTest, RootChildless) {
  layout();

  EXPECT_BOUNDS(ll_, rect_from({}, {100, 100}));
}

TEST_F(LinearLayoutTest, RootWithChild_Horizontal) {
  horizontal();
  auto child = add_child(wc, wc);
  layout();

  EXPECT_BOUNDS(child, rect_from({0, 44}, {12, 12}));
}

TEST_F(LinearLayoutTest, RootWithChild_Vertical) {
  vertical();
  auto child = add_child(wc, wc);
  layout();

  EXPECT_BOUNDS(child, rect_from({44, 0}, {12, 12}));
}

TEST_P(LinearLayoutTestWithOrientation, RootWithChild_MP_MP) {
  auto child = add_child(mp, mp);
  layout();

  EXPECT_BOUNDS(child, rect_from({0, 0}, {100, 100}));
}

TEST_F(LinearLayoutTest, RootWithChild_WC_MP_Horizontal) {
  horizontal();
  auto child = add_child(wc, mp);
  layout();

  EXPECT_BOUNDS(child, rect_from({0, 0}, {12, 100}));
}

TEST_F(LinearLayoutTest, RootWithChild_MP_WC_Vertical) {
  vertical();
  auto child = add_child(mp, wc);
  layout();

  EXPECT_BOUNDS(child, rect_from({0, 0}, {100, 12}));
}

TEST_F(LinearLayoutTest, MultipleWeightless_Horizontal) {
  horizontal();
  auto child1 = add_child(ex(20), wc);
  auto child2 = add_child(wc, mp);
  auto child3 = add_child(ex(40), mp);
  layout();

  EXPECT_BOUNDS(child1, rect_from({0, 44}, {20, 12}));
  EXPECT_BOUNDS(child2, rect_from({20, 0}, {12, 100}));
  EXPECT_BOUNDS(child3, rect_from({32, 0}, {40, 100}));
}

TEST_F(LinearLayoutTest, MultipleWeightless_Vertical) {
  using namespace base::operators;
  vertical();
  auto child1 = add_child(wc, ex(20));
  auto child2 = add_child(mp, wc);
  auto child3 = add_child(wc, ex(40), Gravity::RIGHT | Gravity::BOTTOM);
  layout();

  EXPECT_BOUNDS(child1, rect_from({44, 0}, {12, 20}));
  EXPECT_BOUNDS(child2, rect_from({0, 20}, {100, 12}));
  EXPECT_BOUNDS(child3, rect_from({88, 32}, {12, 40}));
}

TEST_F(LinearLayoutTestWC, MultipleWeightless_Horizontal) {
  horizontal();
  auto child1 = add_child(ex(20), wc);
  auto child2 = add_child(wc, mp);
  auto child3 = add_child(ex(40), mp);
  layout();

  EXPECT_BOUNDS(ll_, rect_from({0, 0}, {72, 12}));

  EXPECT_BOUNDS(child1, rect_from({0, 0}, {20, 12}));
  EXPECT_BOUNDS(child2, rect_from({20, 0}, {12, 12}));
  EXPECT_BOUNDS(child3, rect_from({32, 0}, {40, 12}));
}

TEST_F(LinearLayoutTestWC, MultipleWeightless_Vertical) {
  vertical();
  auto child1 = add_child(wc, ex(20), Gravity::TOP);
  auto child2 = add_child(mp, wc, Gravity::LEFT);
  auto child3 = add_child(mp, ex(40), Gravity::BOTTOM);
  layout();

  EXPECT_BOUNDS(ll_, rect_from({0, 0}, {12, 72}));

  EXPECT_BOUNDS(child1, rect_from({0, 0}, {12, 20}));
  EXPECT_BOUNDS(child2, rect_from({0, 20}, {12, 12}));
  EXPECT_BOUNDS(child3, rect_from({0, 32}, {12, 40}));
}

TEST_F(LinearLayoutTest, MultipleMixed_Horizontal) {
  horizontal();
  auto child1 = add_child(ex(0), wc, 1.f);
  auto child2 = add_child(wc, mp);
  auto child3 = add_child(ex(40), mp, 1.f);
  layout();

  EXPECT_BOUNDS(child1, rect_from({0, 44}, {44, 12}));
  EXPECT_BOUNDS(child2, rect_from({44, 0}, {12, 100}));
  EXPECT_BOUNDS(child3, rect_from({56, 0}, {44, 100}));
}

TEST_F(LinearLayoutTest, MultipleMixed_Vertical) {
  vertical();
  auto child1 = add_child(wc, wc, 2.f, Gravity::RIGHT);
  auto child2 = add_child(mp, ex(50));
  auto child3 = add_child(mp, ex(10), 3.f);
  layout();

  EXPECT_BOUNDS(child1, rect_from({88, 0}, {12, 20}));
  EXPECT_BOUNDS(child2, rect_from({0, 20}, {100, 50}));
  EXPECT_BOUNDS(child3, rect_from({0, 70}, {100, 30}));
}

TEST_F(LinearLayoutTestWC, MultipleMixed_Horizontal) {
  horizontal();
  auto child1 = add_child(ex(0), wc, 1.f);
  auto child2 = add_child(wc, mp);
  auto child3 = add_child(ex(40), mp, 1.f);
  layout();

  EXPECT_BOUNDS(ll_, rect_from({0, 0}, {52, 12}));

  EXPECT_BOUNDS(child1, rect_from({0, 0}, {0, 12}));
  EXPECT_BOUNDS(child2, rect_from({0, 0}, {12, 12}));
  EXPECT_BOUNDS(child3, rect_from({12, 0}, {40, 12}));
}

TEST_F(LinearLayoutTestWC, MultipleMixed_Oversize_Horizontal) {
  horizontal();
  auto child1 = add_child(ex(40), wc, 1.f);
  auto child2 = add_child(ex(40), mp);
  auto child3 = add_child(ex(40), mp, 1.f);
  layout();

  EXPECT_BOUNDS(ll_, rect_from({0, 0}, {100, 12}));

  EXPECT_BOUNDS(child1, rect_from({0, 0}, {40, 12}));
  EXPECT_BOUNDS(child2, rect_from({40, 0}, {40, 12}));
  EXPECT_BOUNDS(child3, rect_from({80, 0}, {40, 12}));
}

TEST_F(LinearLayoutTest, MultipleMixed_Oversize_Horizontal) {
  horizontal();
  auto child1 = add_child(ex(40), wc, 1.f);
  auto child2 = add_child(ex(120), mp);
  auto child3 = add_child(ex(40), mp, 1.f);
  layout();

  EXPECT_BOUNDS(child1, rect_from({0, 44}, {0, 12}));
  EXPECT_BOUNDS(child2, rect_from({0, 0}, {120, 100}));
  EXPECT_BOUNDS(child3, rect_from({120, 0}, {0, 100}));
}

INSTANTIATE_TEST_SUITE_P(LinearLayoutTests,
                         LinearLayoutTestWithOrientation,
                         ::testing::Values(LinearLayout::HORIZONTAL,
                                           LinearLayout::VERTICAL));
