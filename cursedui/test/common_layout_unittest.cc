// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/test/test_harness.hpp"
#include "cursedui/views/frame_layout.hpp"
#include "cursedui/views/linear_layout.hpp"
#include "third-party/googletest/gtest.hpp"

using namespace cursedui;
using namespace cursedui::gfx;
using namespace cursedui::view;

using LayoutClasses = ::testing::Types<FrameLayout, LinearLayout>;

// gtest hasn't thought of anything better than to use RTTI to obtain template parameters'
// names for typed tests. We forbid rtti yet we can actually hack in and provide
// specializations for typename getting function.
#define GTEST_DEFINE_TYPE_NAME(T)                   \
  template <>                                       \
  std::string testing::internal::GetTypeName<T>() { \
    return #T;                                      \
  }

GTEST_DEFINE_TYPE_NAME(FrameLayout);
GTEST_DEFINE_TYPE_NAME(LinearLayout);

template <class T>
class CommonLayoutTest : public ::testing::Test {
 protected:
  base::ref_ptr<FrameLayout> root_ = base::make_ref_ptr<FrameLayout>();
  base::ref_ptr<T> group_ = base::make_ref_ptr<T>();
  base::ref_ptr<View> view_ = test::make_view();

  CommonLayoutTest() {
    add_child(root_, group_, LayoutMatchParent{}, LayoutMatchParent{});
  }
};

TYPED_TEST_SUITE(CommonLayoutTest, LayoutClasses);

TYPED_TEST(CommonLayoutTest, MatchParentMatchParent) {
  add_child(this->group_, this->view_, LayoutMatchParent{}, LayoutMatchParent{});
  auto bounds = rect_from({0, 0}, {100, 100});

  this->root_->layout_as_root(bounds);

  EXPECT_BOUNDS(this->view_, bounds);
}

TYPED_TEST(CommonLayoutTest, MatchParentWrapContent) {
  add_child(this->group_, this->view_, LayoutMatchParent{}, LayoutWrapContent{});

  this->root_->layout_as_root(rect_from({0, 0}, {100, 100}));

  EXPECT_BOUNDS(this->view_, rect_from({0, 44}, {100, 12}));
}
