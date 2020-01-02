
#include "cursedui/view_root.hpp"
#include "cursedui/context.hpp"

#include "cursedui/text_view.hpp"
#include "cursedui/linear_layout.hpp"


[[noreturn]]
int main() {
  using namespace cursedui;

  Context context;

  auto lin_layout = base::ref_ptr(new view::LinearLayout());
  auto lin_layout2 = base::ref_ptr(new view::LinearLayout());
  lin_layout2->set_orientation(view::LinearLayout::VERTICAL);

  auto view1 = base::ref_ptr(new view::TextView());
  auto view2 = base::ref_ptr(new view::TextView());
  view1->set_text(L"Test ◕ string");
  view2->set_text(L"◕ Prod ◕ string ◕ long");

  lin_layout->add_child(view1);
  lin_layout->add_child(lin_layout2);
  lin_layout->add_child(view2);

  auto* lp1 = (view::LinearLayout::LayoutParams*) view1->layout_params();
  lp1->set_weight(1.0f);
  lp1->set_height_layout_spec(view::LayoutMatchParent{});

  auto* lp2 = (view::LinearLayout::LayoutParams*) view2->layout_params();
  lp2->set_weight(0.5f);
  lp2->set_height_layout_spec(view::LayoutMatchParent{});

  auto view3 = base::ref_ptr(new view::TextView());
  auto view4 = base::ref_ptr(new view::TextView());
  view3->set_text(L"Test ◕ string");
  view4->set_text(L"◕ Prod ◕ string ◕ long");
  lin_layout2->add_child(view3);
  lin_layout2->add_child(view4);
  auto* lp3 = (view::LinearLayout::LayoutParams*) view3->layout_params();
  lp3->set_weight(1.0f);
  lp3->set_width_layout_spec(view::LayoutMatchParent{});
  auto* lp4 = (view::LinearLayout::LayoutParams*) view4->layout_params();
  lp4->set_weight(0.5f);
  lp4->set_width_layout_spec(view::LayoutMatchParent{});

  auto* ll = (view::LinearLayout::LayoutParams*) lin_layout2->layout_params();
  ll->set_weight(0.5f);

  view::ViewRoot view_root{&context};
  view_root.set_view_root(lin_layout);

  context.run(&view_root);
}
