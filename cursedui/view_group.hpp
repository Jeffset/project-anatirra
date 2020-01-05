//
// Created by jeffset on 12/15/19.
//

#ifndef CURSEDUI_VIEW_GROUP_HPP
#define CURSEDUI_VIEW_GROUP_HPP

#include "base/macro.hpp"
#include "cursedui/dim.hpp"
#include "cursedui/view.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace cursedui::view {

class LayoutParams {
 public:
  GETTER LayoutSpec width_layout_spec() const noexcept;
  GETTER LayoutSpec height_layout_spec() const noexcept;

  void set_width_layout_spec(const LayoutSpec& spec) noexcept;
  void set_height_layout_spec(const LayoutSpec& spec) noexcept;

  virtual ~LayoutParams() noexcept;

  GETTER virtual std::string_view tag() const noexcept = 0;

 protected:
  LayoutParams(const LayoutSpec& width, const LayoutSpec& height) noexcept;

 private:
  LayoutSpec width_;
  LayoutSpec height_;
};

class ViewGroup : public View {
 public:
  ViewGroup() noexcept;
  ~ViewGroup() noexcept override;

  void add_child(base::ref_ptr<View> child);
  void remove_child(const base::ref_ptr<View>& child);

  GETTER int child_count() const noexcept;
  GETTER base::ref_ptr<View> get_child(int index);

 protected:
  void on_draw(render::Canvas& canvas) override;
  void on_measure(const MeasureSpec& width_spec,
                  const MeasureSpec& height_spec) override = 0;
  void on_layout() override = 0;

  virtual std::unique_ptr<LayoutParams> create_layout_params() = 0;
  virtual bool check_layout_params(LayoutParams* params) = 0;

 private:
  std::vector<base::ref_ptr<View>> children_;
};

}  // namespace cursedui::view

#endif  // CURSEDUI_VIEW_GROUP_HPP
