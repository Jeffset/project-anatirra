//
// Created by jeffset on 12/8/19.
//

#include "cursedui/view.hpp"

#include "base/util.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/rendering.hpp"
#include "cursedui/view_group.hpp"

#include <cassert>

namespace cursedui::view {

struct View::ViewImpl {};

void View::measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec) {
  measured_size_.reset();
  auto double_border_width = border() ? border_->border_width() * 2 : 0;
  if (double_border_width > 0) {
    on_measure(shrink_measure_spec(width_spec, double_border_width),
               shrink_measure_spec(height_spec, double_border_width));
  } else {
    on_measure(width_spec, height_spec);
  }

  assert(measured_size_.has_value());

  if (double_border_width > 0) {
    measured_size_->width += double_border_width;
    measured_size_->height += double_border_width;
  }

  // Assert that measuring is done according to specs.
  // NOTE: Think: is this actually right idea to enforce it?
  std::visit(base::overloaded{
                 [this](const MeasureExactly& exactly) {
                   if (measured_size_->width != exactly.dim)
                     throw measure_spec_violated_exception();
                 },
                 [this](const MeasureAtMost& at_most) {
                   if (measured_size_->width > at_most.dim)
                     throw measure_spec_violated_exception();
                 },
                 [](MeasureUnlimited) {},
             },
             width_spec);
  std::visit(base::overloaded{
                 [this](const MeasureExactly& exactly) {
                   if (measured_size_->height != exactly.dim)
                     throw measure_spec_violated_exception();
                 },
                 [this](const MeasureAtMost& at_most) {
                   if (measured_size_->height > at_most.dim)
                     throw measure_spec_violated_exception();
                 },
                 [](MeasureUnlimited) {},
             },
             height_spec);
}

void View::layout(const gfx::Rect& area) {
  bounds_ = area;
  if (background_)
    background_->set_bounds(inner_bounds());
  if (border_)
    border_->set_bounds(outer_bounds());
  on_layout();
}

void View::colorize(render::ColorPalette& palette) {
  if (background_)
    background_->colorize(palette);
  if (border_)
    border_->colorize(palette);
  on_colorize(palette);
}

render::BgColorState View::draw(render::Canvas& canvas) {
  return on_draw(canvas);
}

View::~View() = default;

View::View() : background_(nullptr), border_(new BorderDrawable()), PIMPL_INIT(View) {}

base::nullable_ptr<LayoutParams> View::layout_params() const noexcept {
  return layout_params_.get();
}

void View::set_layout_params(std::unique_ptr<LayoutParams> layout_params) {
  layout_params_ = std::move(layout_params);
}

void View::set_background(std::unique_ptr<Drawable> drawable) {
  background_ = std::move(drawable);
}

void View::set_background_color(render::ColorDescr color) {
  background_ = std::make_unique<SolidColorDrawable>(color);
}

base::nullable_ptr<Drawable> View::background() {
  return background_.get();
}

void View::set_measured_size(const gfx::Size& measured_size) {
  measured_size_ = measured_size;
}

void View::on_measure(const MeasureSpec& width_spec, const MeasureSpec& height_spec) {
  constexpr auto measurer = base::overloaded{
      [](const MeasureExactly& spec) { return spec.dim; },
      [](const auto&) { return 0; },
  };
  set_measured_size(
      {std::visit(measurer, width_spec), std::visit(measurer, height_spec)});
}

void View::on_layout() {}

void View::on_colorize(render::ColorPalette&) {}

render::BgColorState View::on_draw(render::Canvas& canvas) {
  if (!outer_bounds().has_area()) {
    return render::BgColorState{};
  }

  render::BgColorState bg;
  if (background_)
    bg = background_->draw(canvas);

  if (border_)
    border_->draw(canvas);

  return bg;
}

gfx::Rect View::inner_bounds() const noexcept {
  // FIXME: a bit too heavy for a getter, isn't it?
  return border_ ? gfx::shrink(bounds_.value(), border_->border_width())
                 : bounds_.value();
}

gfx::Rect View::outer_bounds() const noexcept {
  return bounds_.value();
}

const char* measure_spec_violated_exception::what() const noexcept {
  return "Measure spec has been violated";
}

}  // namespace cursedui::view
