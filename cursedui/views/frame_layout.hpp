#ifndef FRAMELAYOUT_HPP
#define FRAMELAYOUT_HPP

#include "cursedui/view_group.hpp"

namespace cursedui::view {

class FrameLayout : public ViewGroup {
 public:
  void on_measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec) override;
  void on_layout() override;
};

}  // namespace cursedui::view

#endif  // FRAMELAYOUT_HPP
