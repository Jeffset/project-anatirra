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

#pragma once

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
  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override {
    return {std::visit(make_measurer(preferred_size_.width), width_spec),
            std::visit(make_measurer(preferred_size_.height), height_spec)};
  }
};

inline auto make_view(gfx::Size preferred_size = {10, 10}) {
  return base::make_ref_ptr<test::ViewForTest>(preferred_size);
}

#define EXPECT_BOUNDS(view, bounds) EXPECT_EQ(view->outer_bounds(), (bounds))

}  // namespace cursedui::view::test