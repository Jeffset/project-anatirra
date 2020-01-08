//
// Created by jeffset on 12/9/19.
//

#include "cursedui/context.hpp"

#include "cursedui/rendering.hpp"
#include "tickit.h"

#include <clocale>
#include <iostream>

namespace cursedui {

struct Context::ContextImpl {
  Tickit* tickit_;
  TickitWindow* window_;
  TickitTerm* term_;

  Delegate* delegate_;

  static int render_cb(TickitWindow*, TickitEventFlags, void* info, void* user) {
    auto* impl = static_cast<ContextImpl*>(user);
    auto* render_info = static_cast<TickitExposeEventInfo*>(info);
    render::Canvas canvas{render_info->rb};
    impl->delegate_->render(canvas);
    return 1;
  }

  static int resize_cb(TickitTerm*, TickitEventFlags, void*, void* user) {
    auto* impl = static_cast<ContextImpl*>(user);
    tickit_window_expose(impl->window_, nullptr);
    return 1;
  }
};

Context::Context() : impl_(new ContextImpl()) {
  std::setlocale(LC_ALL, "");

  impl_->tickit_ = tickit_new_stdio();
  impl_->window_ = tickit_get_rootwin(impl_->tickit_);
  impl_->term_ = tickit_get_term(impl_->tickit_);

  tickit_term_set_utf8(impl_->term_, true);

  tickit_window_bind_event(impl_->window_, TICKIT_WINDOW_ON_EXPOSE, TICKIT_BIND_FIRST,
                           &ContextImpl::render_cb, impl_.get());
  tickit_term_bind_event(impl_->term_, TICKIT_TERM_ON_RESIZE, TICKIT_BIND_FIRST,
                         &ContextImpl::resize_cb, impl_.get());

  int colors;
  tickit_term_getctl_int(impl_->term_, TICKIT_TERMCTL_COLORS, &colors);
  std::cerr << "Colors: " << colors << std::endl;
}

Context::~Context() {
  tickit_window_close(impl_->window_);
  tickit_unref(impl_->tickit_);
}

gfx::Size Context::screen_size() const {
  gfx::Size size{};
  tickit_term_get_size(impl_->term_, &size.height, &size.width);
  return size;
}

void Context::run(Delegate* delegate) {
  impl_->delegate_ = delegate;
  tickit_run(impl_->tickit_);
}

}  // namespace cursedui
