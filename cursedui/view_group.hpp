//
// Created by jeffset on 12/15/19.
//

#ifndef CURSES_DEMO_VIEW_GROUP_HPP
#define CURSES_DEMO_VIEW_GROUP_HPP

#include "view.hpp"
#include "macro.hpp"
#include "gfx.hpp"

#include <vector>
#include <memory>
#include <string_view>


namespace cursedui::view {

class LayoutParams {
 public:
  GETTER LayoutSpec width_layout_spec() const;
  GETTER LayoutSpec height_layout_spec() const;

  void set_width_layout_spec(const LayoutSpec& spec);
  void set_height_layout_spec(const LayoutSpec& spec);

  virtual ~LayoutParams();

  GETTER virtual std::string_view tag() const = 0;

 protected:
  LayoutParams(const LayoutSpec& width, const LayoutSpec& height);

 private:
  LayoutSpec width_;
  LayoutSpec height_;
};

class ViewGroup : public View {
 public:
  ViewGroup();
  ~ViewGroup() override;

  void add_child(base::ref_ptr<View> child);
  void remove_child(const base::ref_ptr<View>& child);

  GETTER int child_count() const;
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

}

#endif //CURSES_DEMO_VIEW_GROUP_HPP
