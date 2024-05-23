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

#include "cursedui/view_group.hpp"

#include "cursedui/config.hpp"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace cursedui::view {

class CURSEDUI_PUBLIC LinearLayout : public ViewGroup {
 public:
  class LayoutParams : public view::LayoutParams {
   public:
    static const char* TAG;

    LayoutParams(LayoutSpec width,
                 LayoutSpec height,
                 base::EnumFlags<gfx::Gravity> gravity = gfx::Gravity::CENTER) noexcept;
    LayoutParams(LayoutSpec width,
                 LayoutSpec height,
                 float weight,
                 base::EnumFlags<gfx::Gravity> gravity = gfx::Gravity::CENTER) noexcept;

    void set_weight(float weight);
    GETTER std::optional<float> weight() const { return weight_; }

    void set_no_weight();

   private:
    GETTER std::string_view tag() const noexcept override;

   private:
    std::optional<float> weight_;
  };

  enum Orientation {
    HORIZONTAL = 0,
    VERTICAL = 1,
  };

  LinearLayout() noexcept;
  ~LinearLayout() noexcept override;

  GETTER Orientation orientation() const noexcept { return orientation_; }

  void set_orientation(Orientation orientation) noexcept;

  std::unique_ptr<view::LayoutParams> create_layout_params() const noexcept override;
  bool check_layout_params(view::LayoutParams* params) const noexcept override;

 protected:
  gfx::Size on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;
  void propagate_needs_layout_mark(const View* child) override;

 private:
  Orientation orientation_;
  std::vector<View*> match_parent_children_tmp_;
};

}  // namespace cursedui::view

