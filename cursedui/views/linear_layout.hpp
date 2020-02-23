//
// Created by jeffset on 12/15/19.
//

#ifndef CURSEDUI_LINEAR_LAYOUT_HPP
#define CURSEDUI_LINEAR_LAYOUT_HPP

#include "cursedui/view_group.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace cursedui::view {

class LinearLayout : public ViewGroup {
 public:
  class LayoutParams : public view::LayoutParams {
   public:
    static const char* TAG;

    LayoutParams(const LayoutSpec& width, const LayoutSpec& height);

    GETTER std::optional<float> weight() const { return weight_; }

    void set_weight(float weight);
    void set_no_weight();

   private:
    GETTER std::string_view tag() const noexcept override;

   private:
    std::optional<float> weight_;
  };

  enum Orientation : unsigned char { HORIZONTAL, VERTICAL };

  LinearLayout() noexcept;
  ~LinearLayout() noexcept override;

  GETTER Orientation orientation() const noexcept { return orientation_; }

  void set_orientation(Orientation orientation) noexcept;

  std::unique_ptr<view::LayoutParams> create_layout_params() const noexcept override;
  bool check_layout_params(view::LayoutParams* params) const noexcept override;

 protected:
  void on_measure(MeasureSpec width_spec, MeasureSpec height_spec) override;
  void on_layout() override;

 private:
  Orientation orientation_;
};

}  // namespace cursedui::view

#endif  // CURSEDUI_LINEAR_LAYOUT_HPP
