//
// Created by jeffset on 12/8/19.
//

#include "view.hpp"

#include "view_group.hpp"
#include "rendering.hpp"
#include "util.hpp"

#include <cassert>


namespace cursedui::view {

const gfx::dim_t BORDER_WIDTH = 1;
const gfx::dim_t D_BORDER_WIDTH = BORDER_WIDTH * 2;

struct View::ViewImpl {
};

void View::measure(const MeasureSpec& width_spec,
                   const MeasureSpec& height_spec) {

  measured_size_.reset();
  if (border_style_)
    on_measure(shrink_measure_spec(width_spec, D_BORDER_WIDTH),
               shrink_measure_spec(height_spec, D_BORDER_WIDTH));
  else
    on_measure(width_spec, height_spec);

  assert(measured_size_.has_value());

  if (border_style_) {
    measured_size_->width += D_BORDER_WIDTH;
    measured_size_->height += D_BORDER_WIDTH;
  }

  // Assert that measuring is done according to specs.
  // TODO: Think: is this actually right idea to enforce it?
  std::visit(base::overloaded{
      [this](const MeasureExactly& exactly) {
        assert(measured_size_->width == exactly.dim);
      },
      [this](const MeasureAtMost& at_most) {
        assert(measured_size_->width <= at_most.dim);
      },
      [](MeasureUnlimited) {},
  }, width_spec);
  std::visit(base::overloaded{
      [this](const MeasureExactly& exactly) {
        assert(measured_size_->height == exactly.dim);
      },
      [this](const MeasureAtMost& at_most) {
        assert(measured_size_->height <= at_most.dim);
      },
      [](MeasureUnlimited) {},
  }, height_spec);
}

void View::layout(const gfx::Rect& area) {
  bounds_ = area;
  on_layout();
}

void View::draw(render::Canvas& canvas) {
  on_draw(canvas);
}

View::~View() = default;

View::View() :
    background_(L' '),
    border_style_(&render::BorderStyles::Single),

    impl_(new ViewImpl()) {}

void View::set_background(wchar_t b) {
  background_ = b;
}

void View::set_layout_params(std::unique_ptr<LayoutParams> layout_params) {
  layout_params_ = std::move(layout_params);
}

void View::set_measured_size(const gfx::Size& measured_size) {
  measured_size_ = measured_size;
}

void View::on_measure(const MeasureSpec& width_spec,
                      const MeasureSpec& height_spec) {
  constexpr auto measurer = base::overloaded{
      [](const MeasureExactly& spec) { return spec.dim; },
      [](const auto&) { return 0; },
  };
  set_measured_size({
                        .width = std::visit(measurer, width_spec),
                        .height = std::visit(measurer, height_spec)
                    });
}

void View::on_layout() {}

void View::on_draw(render::Canvas& canvas) {
  if (!outer_bounds().has_area()) {
    return;
  }

  if (inner_bounds().has_area()) {
    render::fill(canvas, background_, inner_bounds());
  }

  if (border_style_) {
    render::border(canvas, outer_bounds(), *border_style_);
  }
}

gfx::Rect View::inner_bounds() const {
  return border_style_ ? gfx::shrink(bounds_.value(), BORDER_WIDTH) :
         bounds_.value();
}

gfx::Rect View::outer_bounds() const {
  return bounds_.value();
}

void View::set_border_style(render::BorderStyle const* border_style) {
  border_style_ = border_style;
}

}