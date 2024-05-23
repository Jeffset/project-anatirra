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

#include "avada/color.hpp"
#include "base/debug/debug.hpp"
#include "base/run_loop.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/view_tree_host.hpp"
#include "cursedui/views/frame_layout.hpp"
#include "cursedui/views/linear_layout.hpp"
#include "cursedui/views/scroll_view.hpp"
#include "cursedui/views/text_view.hpp"

#include <fstream>
#include <iterator>
#include <locale>
#include <memory>

int main() {
  using namespace cursedui;
  using namespace base::operators;

  std::setlocale(LC_ALL, "");

  base::debug::LoggerToStdErr logger;
  base::debug::setup_logging(&logger);

  base::RunLoop main_loop;
  base::RunLoop::set_main(&main_loop);

  auto root = base::make_ref_ptr<view::FrameLayout>();
  root->set_debug_name("root-frame");

  auto scroll_view = base::make_ref_ptr<view::ScrollView>();
  scroll_view->border().set_style(BorderDrawable::Style::SINGLE);

  auto lin_layout = base::make_ref_ptr<view::LinearLayout>();
  lin_layout->set_debug_name("root-hor-linear-layout");
  lin_layout->set_layout_params(std::make_unique<view::LayoutParams>(
      view::LayoutMatchParent{}, view::LayoutMatchParent{}));
  root->add_child(lin_layout);

  auto overlay = base::make_ref_ptr<view::TextView>();
  overlay->set_layout_params(std::make_unique<view::LayoutParams>(
      view::LayoutWrapContent{}, view::LayoutMatchParent{}));
  overlay->set_text("Hello world!");
  overlay->set_background_color(avada::render::ColorRGB{255, 0, 255, 128});
  overlay->set_debug_name("overlay");
  root->add_child(overlay);

  lin_layout->set_background_color(avada::render::ColorRGB{33, 20, 20});
  auto lin_layout2 = base::make_ref_ptr<view::LinearLayout>();
  lin_layout2->set_orientation(view::LinearLayout::VERTICAL);
  lin_layout2->set_debug_name("child-ver-linear-layout");

  auto view1 = base::make_ref_ptr<view::TextView>();
  auto view2 = base::make_ref_ptr<view::TextView>();
  view1->set_gravity(gfx::Gravity::TOP | gfx::Gravity::LEFT);
  view1->set_debug_name("text-view-A");
  std::ifstream ifs{"/home/jeffset/text.txt"};
  ifs.imbue(std::locale(""));
  std::string string{std::istreambuf_iterator{ifs}, {}};
  LOG() << "SOURCE TEXT: " << string << '\n';
  // view1->set_text(wstring + wstring);
  view1->set_multiline(true);
  view1->border().set_style(BorderDrawable::Style::NO_BORDER);

  view1->set_background_color(avada::render::ColorRGB{100, 34, 40});
  view1->set_text_color(avada::render::ColorRGB{0, 255, 255});
  view2->set_text("◕ Prod ◕ string ◕ long B");
  view2->set_debug_name("text-view-B");

  view2->border().set_style(BorderDrawable::Style::DOUBLE);

  lin_layout->add_child(scroll_view);
  scroll_view->add_child(view1);
  lin_layout->add_child(lin_layout2);
  lin_layout->add_child(view2);

  auto* lp_scroll = (view::LinearLayout::LayoutParams*)scroll_view->layout_params().get();
  lp_scroll->set_weight(1.0f);
  lp_scroll->set_height_layout_spec(view::LayoutMatchParent{});

  auto* lp1 = (view::LayoutParams*)view1->layout_params().get();
  lp1->set_width_layout_spec(view::LayoutMatchParent{});
  lp1->set_gravity(gfx::Gravity::TOP | gfx::Gravity::LEFT);

  auto* lp2 = (view::LinearLayout::LayoutParams*)view2->layout_params().get();
  lp2->set_weight(0.5f);
  lp2->set_height_layout_spec(view::LayoutMatchParent{});

  auto view3 = base::make_ref_ptr<view::TextView>();
  view3->border().set_color(avada::render::ColorRGB{0, 255, 200});
  auto view4 = base::make_ref_ptr<view::TextView>();
  view4->border().set_color(avada::render::ColorRGB{0, 50, 18});
  view4->border().set_background_color(avada::render::ColorRGB{128, 255, 255});
  view3->set_text("Test ◕ string");
  view3->set_multiline(true);
  view3->set_text(std::move(string));
  view3->set_debug_name("text-view-file");

  view4->set_text("◕ Prod ◕ string ◕ long C");
  view4->set_debug_name("text-view-C");
  lin_layout2->add_child(view3);
  lin_layout2->add_child(view4);
  auto* lp3 = (view::LinearLayout::LayoutParams*)view3->layout_params().get();
  //  lp3->set_weight(1.0f);
  lp3->set_width_layout_spec(view::LayoutMatchParent{});
  auto* lp4 = (view::LinearLayout::LayoutParams*)view4->layout_params().get();
  // lp4->set_weight(0.5f);
  lp4->set_width_layout_spec(view::LayoutMatchParent{});

  auto* ll = (view::LinearLayout::LayoutParams*)lin_layout2->layout_params().get();
  ll->set_weight(0.5f);

  try {
    ViewTreeHost view_tree_host{root};
    main_loop.run([&view_tree_host]() {
      view_tree_host.tick();
    });
  } catch (base::exception& e) {
    LOG() << e.stack_trace().to_string(e.what());
  }
}
