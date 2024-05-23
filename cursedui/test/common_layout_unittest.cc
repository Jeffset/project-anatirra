/* Copyright 2020-2024 Fedor Ihnatkevich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cursedui/drawable.hpp"
#include "cursedui/test/test_harness.hpp"
#include "cursedui/views/frame_layout.hpp"
#include "cursedui/views/linear_layout.hpp"

#include "gtest/gtest.h"

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
    view_->border().set_style(BorderDrawable::Style::SINGLE);
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
