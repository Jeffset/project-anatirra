//
// Created by jeffset on 12/15/19.
//

#ifndef CURSES_DEMO_LINEAR_LAYOUT_HPP
#define CURSES_DEMO_LINEAR_LAYOUT_HPP

#include "view_group.hpp"

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
    GETTER std::string_view tag() const override;

   private:
    std::optional<float> weight_;
  };

  enum Orientation : unsigned char { HORIZONTAL, VERTICAL };

  LinearLayout();
  ~LinearLayout() override;

  GETTER Orientation orientation() const { return orientation_; }

  void set_orientation(Orientation orientation);

 protected:
  void on_measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec) override;
  void on_layout() override;

 protected:
  std::unique_ptr<view::LayoutParams> create_layout_params() override;
  bool check_layout_params(view::LayoutParams* params) override;

 private:
  Orientation orientation_;
};

}  // namespace cursedui::view

#endif  // CURSES_DEMO_LINEAR_LAYOUT_HPP
