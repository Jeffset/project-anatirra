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

#include "cursedui/view_tree_host.hpp"

#include "base/run_loop.hpp"
#include "base/util.hpp"
#include "base/weak_ref.hpp"
#include "cursedui/canvas.hpp"
#include "cursedui/view.hpp"
#include "cursedui/view_group.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

namespace cursedui {

namespace {

template <class V>
class ViewTreeVisitor : public view::ViewTreeVisitor {
 public:
  ViewTreeVisitor(V&& v) : visitor_(std::forward<V>(v)) {}

  view::VisitResult visit(view::View* view) const final { return visitor_(view); }

 private:
  V visitor_;
};

}  // namespace

ViewTreeHost::ViewTreeHost(base::ref_ptr<view::View> root)
    : root_(std::move(root)),
      root_size_{avada_.get_columns(), avada_.get_rows()},
      need_root_resize_{true} {
  root_->set_tree_host(this);

  view_tree_routine();
}

void ViewTreeHost::set_focused_view(base::ref_ptr<view::View> focused_view) noexcept {
  if (focused_view_ == focused_view)
    return;

  if (focused_view == nullptr) {
    focused_view_ = nullptr;
  } else {
    ASSERT(focused_view->tree_host() == this);
    focused_view_ = base::weak_ref(focused_view);
    LOG() << "ViewTreeHost: view '" << focused_view->debug_name() << "' focused.";
  }
}

base::ref_ptr<view::View> ViewTreeHost::focused_view() const noexcept {
  return focused_view_.lock();
}

void ViewTreeHost::work_routine() {
  using namespace avada::input;

  avada::input::Event event;

  try {
    event = avada_.poll_event(1ms);
  } catch (avada::input::unparsed_exception& e) {
    LOG() << "Unparsed input! See: " << e.what();
    event = ServiceEvent::IDLE;
  }

  const auto visitor = base::overloaded{
      [this](const ResizeEvent& re) {
        gfx::Size new_size{re.columns, re.rows};
        if (new_size == root_size_)
          return;
        root_size_ = new_size;
        need_root_resize_ = true;
      },
      [this](const KeyboardEvent& key) {
        if (key == KeyboardKey::TAB) {
          // TODO: focused view switching logic
          return;
        }
        if (key == KeyboardEvent{L'q', KeyboardEvent::CTRL}) {
          base::RunLoop::main().exit_when_idle();
          return;
        }

        if (keyboard_handler_ && keyboard_handler_(key)) {
          // Handled by global handler
          return;
        }

        if (auto view = focused_view_.lock()) {
          view->on_key_event(key);
        }
      },
      [this](const MouseEvent& mouse) { root_->dispatch_mouse_event(mouse); },
      [](ServiceEvent se) {
        ASSERT(se == ServiceEvent::IDLE);
        // nothing else here
      }};
  std::visit(visitor, event);

  if (base::RunLoop::current().exit_requested())
    return;

  animation_host_.tick();

  view_tree_routine();
}

void ViewTreeHost::view_tree_routine() {
  paint::Canvas canvas(avada_.render_buffer());
  paint::Region paint_region;
  layout_tree(paint_region);
  if (paint_tree(paint_region, canvas)) {
    avada_.render();
  }
}

void ViewTreeHost::tick() {
  // view_tree_routine();
  work_routine();
}

void ViewTreeHost::layout_tree(paint::Region& repaint_region) {
  if (need_root_resize_) {
    auto bounds = gfx::rect_from({}, root_size_);
    // Mark root as needing size layout to keep internal invariants intact.
    root_->mark_needs_layout(view::NeedsLayout::SIZE);
    root_->layout_as_root(bounds);
    repaint_region.add(bounds);
    need_root_resize_ = false;
    return;
  }

  std::unordered_map<view::View*, gfx::Rect> old_bounds;

  std::vector<view::View*> roots_needing_layout;

  const auto scan_visitor = ViewTreeVisitor{
      [&roots_needing_layout](view::View* view) {
        if (view->needs_layout()) {
          roots_needing_layout.push_back(view);
          return view::VisitResult::STOP_VISIT;
        }
        return view::VisitResult::CONTINUE_VISIT;
      },
  };

  // Step I: obtain all sub-roots of the view hierarchy that explicitily need layout
  root_->visit_down(scan_visitor);

  if (roots_needing_layout.empty())
    return;

  // Step II: propagate layout need from every root up the tree.
  const auto propagate_visitor = ViewTreeVisitor{
      [&old_bounds](view::View* view) {
        old_bounds[view] = view->outer_bounds();
        if (auto* parent = view->get_parent().get_nullable()) {
          parent->propagate_needs_layout_mark(view);
          return parent->needs_layout() ? view::VisitResult::CONTINUE_VISIT
                                        : view::VisitResult::STOP_VISIT;
        }
        return view::VisitResult::STOP_VISIT;
      },
  };
  for (auto& view : roots_needing_layout) {
    view->visit_up(propagate_visitor);
  }

  do {
    // Step III: re-scan the tree to obtain the final set of roots needing layout.
    roots_needing_layout.clear();
    root_->visit_down(scan_visitor);

    // Step IV: Perform actual layout on every final detected root.
    for (auto* view : roots_needing_layout) {
      LOG() << "ViewTreeHost layout: layouting " << view->debug_name();
      view->relayout();
    }
  } while (!roots_needing_layout.empty());

  for (auto& [view, bounds] : old_bounds)
    if (bounds != view->outer_bounds())
      repaint_region.add(bounds);
}

bool ViewTreeHost::paint_tree(paint::Region& paint_region, paint::Canvas& canvas) {
  std::vector<view::View*> roots_to_paint;
  root_->visit_down(ViewTreeVisitor{
      [&roots_to_paint](view::View* view) {
        if (view->needs_paint()) {
          roots_to_paint.push_back(view);
          return view::VisitResult::STOP_VISIT;
        }
        return view::VisitResult::CONTINUE_VISIT;
      },
  });

  if (roots_to_paint.empty())
    return false;

  for (auto* view : roots_to_paint)
    paint_region.add(view->outer_bounds());
  auto scoped_clip_handle = canvas.push_clip(paint_region);
  root_->draw(canvas);
  return true;
}

}  // namespace cursedui
