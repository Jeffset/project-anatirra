// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_RUN_LOOP
#define ANATIRRA_BASE_RUN_LOOP

#include "base/macro.hpp"
#include "base/weak_ref.hpp"

#include "base/config.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <memory_resource>

namespace base {

class BASE_PUBLIC RunLoop {
 public:
  DISABLE_COPY_MOVE(RunLoop);

  using clock_t = std::chrono::steady_clock;
  using duration_t = clock_t::duration;
  using time_point_t = clock_t::time_point;

  class Task : public base::WeakReferencedThreadSafe {
   private:
    friend class RunLoop;
    virtual ~Task() noexcept = default;
    virtual void run() = 0;
  };

  RunLoop() noexcept;
  ~RunLoop() noexcept;

  void exit() noexcept;
  bool exited() const noexcept { return exited_; }

 private:
  void run_impl(Task& loop);
  void post_impl(duration_t delay, base::weak_ref<Task>) noexcept;

  template <class F>
  class TaskImpl final : public Task {
   public:
    explicit TaskImpl(F runnable) : runnable_(std::move(runnable)) {}
    void run() final { runnable_(); }

   private:
    F runnable_;
  };

 public:
  template <class F>
  void run(F&& loop) {
    auto loop_task = TaskImpl(std::forward<F>(loop));
    run_impl(loop_task);
  }

  template <class F>
  base::ref_ptr<Task> post(F&& runnable) noexcept {
    return post({}, std::forward<F>(runnable));
  }

  void repost(const base::ref_ptr<Task>& task) noexcept { repost({}, task); }

  void repost(duration_t delay, const base::ref_ptr<Task>& task) noexcept {
    post_impl(delay, task);
  }

  template <class F>
  base::ref_ptr<Task> post(duration_t delay, F&& runnable) noexcept {
    auto task = base::make_ref_ptr<TaskImpl<F>>(std::forward<F>(runnable));
    post_impl(delay, task);
    return task;
  }

  static RunLoop& current() noexcept;

 private:
  struct TaskNode;
  std::atomic<TaskNode*> tasks_;

  std::pmr::synchronized_pool_resource memory_pool_;
  bool exited_;
  bool running_;
};

}  // namespace base

#endif  // ANATIRRA_BASE_RUN_LOOP
