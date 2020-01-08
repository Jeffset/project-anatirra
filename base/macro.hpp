//
// Created by jeffset on 12/7/19.
//

#ifndef BASE_MACRO_HPP
#define BASE_MACRO_HPP

#define MARK_UNUSED(expr) (void)(expr)

#define DISABLE_COPY_AND_ASSIGN(Class) \
  Class(const Class&) = delete;        \
  Class& operator=(const Class&) = delete

#define DISABLE_MOVE_AND_ASSIGN(Class) \
  Class(Class&&) = delete;             \
  Class& operator=(Class&&) = delete

#define MAKE_FULLY_STATIONAR(Class) \
  DISABLE_COPY_AND_ASSIGN(Class);   \
  DISABLE_MOVE_AND_ASSIGN(Class)

#define NODISCARD [[nodiscard]]

#define GETTER NODISCARD

#define REQUIRES(condition) typename = std::enable_if_t<(condition), void>

#define PIMPL(Class)  \
  struct Class##Impl; \
  std::unique_ptr<Class##Impl> impl_

#define PIMPL_INIT(Class) impl_(std::make_unique<Class##Impl>())

#define PIMPL_DEFINE(Class) struct Class::Class##Impl

#endif  // BASE_MACRO_HPP
