// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_MACRO
#define ANATIRRA_BASE_MACRO

#define MARK_UNUSED(expr) (void)(expr)

#define DISABLE_COPY_AND_ASSIGN(Class) \
  Class(const Class&) = delete;        \
  Class& operator=(const Class&) = delete

#define DISABLE_MOVE_AND_ASSIGN(Class) \
  Class(Class&&) = delete;             \
  Class& operator=(Class&&) = delete

#define DISABLE_COPY_MOVE(Class)  \
  DISABLE_COPY_AND_ASSIGN(Class); \
  DISABLE_MOVE_AND_ASSIGN(Class)

#define NODISCARD [[nodiscard]]

#define GETTER NODISCARD

#define NONNULL [[gnu::__nonnull__]]

#define REQUIRES(condition) typename = std::enable_if_t<(condition), void>

#define PIMPL(Class)  \
  struct Class##Impl; \
  std::unique_ptr<Class##Impl> impl_

#define PIMPL_INIT(Class) impl_(std::make_unique<Class##Impl>())

#define PIMPL_DEFINE(Class) struct Class::Class##Impl

#define LIKELY(cond) __builtin_expect(bool(cond), 1)
#define UNLIKELY(cond) __builtin_expect(bool(cond), 0)

#endif  // ANATIRRA_BASE_MACRO
