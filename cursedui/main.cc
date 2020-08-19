// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "avada/color.hpp"
#include "base/debug/debug.hpp"
#include "cursedui/drawable.hpp"
#include "cursedui/view_tree_host.hpp"
#include "cursedui/views/frame_layout.hpp"
#include "cursedui/views/linear_layout.hpp"
#include "cursedui/views/text_view.hpp"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iterator>
#include <locale>
#include <unordered_map>
#include <vector>

class Factory {
 public:
  base::ref_ptr<cursedui::view::View> create() {
    auto view = base::make_ref_ptr<cursedui::view::TextView>();
    weaks_.emplace_back(view);
    weaks_.push_back(base::weak_ref(view));
    return view;
  }

  int weak_count() const {
    return std::count_if(std::begin(weaks_), std::end(weaks_),
                         [](const auto& ptr) { return ptr.get(); });
  }

 private:
  std::vector<base::weak_ref<cursedui::view::View>> weaks_;
};

[[noreturn]] int main() {
  using namespace cursedui;
  using namespace base::operators;

  std::setlocale(LC_ALL, "");

  base::debug::LoggerToStdErr logger;
  base::debug::setup_logging(&logger);

  auto root = base::make_ref_ptr<view::FrameLayout>();
  root->set_debug_name("root-frame");

  auto lin_layout = base::make_ref_ptr<view::LinearLayout>();
  lin_layout->set_debug_name("root-hor-linear-layout");
  lin_layout->set_layout_params(std::make_unique<view::LayoutParams>(
      view::LayoutMatchParent{}, view::LayoutMatchParent{}));
  root->add_child(lin_layout);

  auto overlay = base::make_ref_ptr<view::TextView>();
  overlay->set_layout_params(std::make_unique<view::LayoutParams>(
      view::LayoutWrapContent{}, view::LayoutMatchParent{}));
  overlay->set_text(L"Hello world!");
  overlay->set_background_color(avada::render::ColorRGB{255, 0, 255, 128});
  overlay->set_debug_name("overlay");
  root->add_child(overlay);

  lin_layout->set_background_color(avada::render::ColorRGB{33, 20, 20});
  auto lin_layout2 = base::make_ref_ptr<view::LinearLayout>();
  lin_layout2->set_orientation(view::LinearLayout::VERTICAL);
  lin_layout2->set_debug_name("child-ver-linear-layout");

  auto view1 = base::make_ref_ptr<view::TextView>();
  auto view2 = base::make_ref_ptr<view::TextView>();
  view1->set_gravity(gfx::Gravity::TOP | gfx::Gravity::BOTTOM);
  view1->set_text(L"Test ◕ string A");
  view1->set_debug_name("text-view-A");
  std::wifstream ifs{"/home/jeffset/text.txt"};
  ifs.imbue(std::locale(""));
  std::wstring wstring{std::istreambuf_iterator<wchar_t>{ifs}, {}};
  LOG() << "SOURCE TEXT: " << wstring << '\n';
  view1->set_text(wstring);
  view1->set_multiline(true);

  view1->set_background_color(avada::render::ColorRGB{100, 34, 40});
  view1->set_text_color(avada::render::ColorRGB{0, 0, 0});
  view2->set_text(L"◕ Prod ◕ string ◕ long B");
  view2->set_debug_name("text-view-B");

  view2->border().set_style(BorderDrawable::Style::DOUBLE);

  lin_layout->add_child(view1);
  lin_layout->add_child(lin_layout2);
  lin_layout->add_child(view2);

  auto* lp1 = (view::LinearLayout::LayoutParams*)view1->layout_params().get();
  lp1->set_weight(1.0f);
  lp1->set_height_layout_spec(view::LayoutMatchParent{});

  auto* lp2 = (view::LinearLayout::LayoutParams*)view2->layout_params().get();
  lp2->set_weight(0.5f);
  lp2->set_height_layout_spec(view::LayoutMatchParent{});

  auto view3 = base::make_ref_ptr<view::TextView>();
  view3->border().set_color(avada::render::ColorRGB{0, 255, 200});
  auto view4 = base::make_ref_ptr<view::TextView>();
  view4->border().set_color(avada::render::ColorRGB{0, 50, 18});
  view4->border().set_background_color(avada::render::ColorRGB{128, 255, 255});
  view3->set_text(L"Test ◕ string");
  view3->set_multiline(true);
  view3->set_text(std::move(wstring));
  view3->set_debug_name("text-view-file");

  view4->set_text(L"◕ Prod ◕ string ◕ long C");
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

  Factory factory;
  ASSERT(factory.weak_count() == 0);
  {
    auto v = factory.create();
    ASSERT(factory.weak_count() == 2);
  }
  ASSERT(factory.weak_count() == 0);

  try {
    ViewTreeHost{root}.run();
  } catch (base::exception& e) {
    LOG() << e.stack_trace().to_string(e.what());
  }
}
