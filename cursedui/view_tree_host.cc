// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/view_tree_host.hpp"

#include "base/macro.hpp"
#include "base/util.hpp"
#include "cursedui/canvas.hpp"
#include "cursedui/view_group.hpp"

#include <forward_list>
#include <type_traits>
#include <utility>

namespace cursedui {

namespace {

class ScanLayoutVisitor : public view::ViewTreeVisitor {
 public:
  ScanLayoutVisitor(std::forward_list<view::View*>& roots) noexcept : roots_(roots) {}

  VisitResult visit(view::View* view) override {
    if (view->needs_layout()) {
      roots_.push_front(view);
      return STOP_VISIT;
    }
    return CONTINUE_VISIT;
  }

 private:
  std::forward_list<view::View*>& roots_;
};

class PropagateLayoutVisitor : public view::ViewTreeVisitor {
 public:
  VisitResult visit(view::View* view) override {
    if (auto* parent = view->get_parent().get_nullable()) {
      parent->propagate_needs_layout_mark(view);
      return parent->needs_layout() ? CONTINUE_VISIT : STOP_VISIT;
    }
    return STOP_VISIT;
  }
};

}  // namespace

ViewTreeHost::ViewTreeHost(base::ref_ptr<view::View> root) : root_(std::move(root)) {
  root_->set_tree_host(this);
  handle_root_size({avada_.get_columns(), avada_.get_rows()});
}

void ViewTreeHost::set_focused_view(base::ref_ptr<view::View> focused_view) noexcept {
  ASSERT(focused_view->tree_host() == this);
  if (focused_view_ == focused_view)
    return;
  focused_view_ = base::weak_ref(focused_view);
  LOG() << "ViewTreeHost: view focused\n";
}

base::ref_ptr<view::View> ViewTreeHost::focused_view() noexcept {
  return focused_view_.get_ref_ptr();
}

void ViewTreeHost::run() {
  paint::Canvas canvas(avada_.render_buffer());
  bool should_exit = false;

  avada::input::Event event = avada::input::ServiceEvent::IDLE;

  while (true) {
    using namespace avada::input;
    const auto visitor = base::overloaded{
        [this](const ResizeEvent& re) {
          handle_root_size({re.columns, re.rows});
        },
        [this, &should_exit](const KeyboardEvent& key) {
          if (key == KeyboardKey::TAB) {
            // TODO: focused view switching logic
            return;
          }
          if (key == KeyboardEvent{L'q', KeyboardEvent::CTRL}) {
            should_exit = true;
            return;
          }

          if (auto* view = focused_view_.get()) {
            view->on_key_event(key);
          }
        },
        [this](const MouseEvent& mouse) { root_->dispatch_mouse_event(mouse); },
        [](ServiceEvent se) {
          ASSERT(se == ServiceEvent::IDLE);
          // nothing else here
        }};
    std::visit(visitor, event);

    if (should_exit)
      break;

    layout_tree();
    paint_tree(canvas);
    avada_.render();

    try {
      event = avada_.poll_event();
    } catch (avada::input::unparsed_exception& e) {
      LOG() << e.what();
      event = ServiceEvent::IDLE;
    }
  };
}

void ViewTreeHost::layout_tree() {
  std::forward_list<view::View*> roots_needing_layout;
  ScanLayoutVisitor scan_visitor{roots_needing_layout};

  // Step I: obtain all sub-roots of the view hierarchy that explcitily need layout
  root_->visit_down(scan_visitor);

  if (roots_needing_layout.empty())
    return;

  // Step II: propagate layout need from every root up the tree.
  PropagateLayoutVisitor propagate_visitor;
  for (auto& view : roots_needing_layout) {
    view->visit_up(propagate_visitor);
  }

  // Step III: re-scan the tree to obtain the final set of roots needing layout.
  roots_needing_layout.clear();
  root_->visit_down(scan_visitor);

  // Step IV: Perform actual layout on every final detected root.
  for (auto* view : roots_needing_layout) {
    LOG() << "ViewTreeHost layout: layouting " << view->debug_name();
    const auto size = view->size();
    view->measure(view::MeasureExactly{{size.width}},
                  view::MeasureExactly{{size.height}});
    view->layout(gfx::rect_from(view->position(), size));
  }
}

void ViewTreeHost::paint_tree(paint::Canvas& canvas) {
  // TODO: make this incremental like layout.
  root_->draw(canvas);
}

void ViewTreeHost::handle_root_size(const gfx::Size& size) {
  root_->measure(view::MeasureExactly{{size.width}}, view::MeasureExactly{{size.height}});
  auto measured_size = root_->measured_size();
  gfx::Rect bounds = gfx::rect_from({0, 0}, measured_size);
  root_->layout(bounds);
}

}  // namespace cursedui
