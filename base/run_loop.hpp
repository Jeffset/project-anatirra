// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#ifndef ANATIRRA_BASE_RUN_LOOP
#define ANATIRRA_BASE_RUN_LOOP

#include "base/macro.hpp"
#include "base/weak_ref.hpp"

#include "base_config.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <list>

namespace base {

class BASE_PUBLIC RunLoop {
 public:
  DISABLE_COPY_MOVE(RunLoop);

  using run_t = std::function<void()>;
  using clock_t = std::chrono::steady_clock;
  using duration_t = clock_t::duration;
  using time_point_t = clock_t::time_point;

  RunLoop() noexcept;
  ~RunLoop() noexcept;

  void run(run_t loop);

  struct Task final : public base::WeakReferencedThreadSafe {
    run_t runnable;
    Task(run_t runnable) : runnable(std::move(runnable)) {}
  };

  void exit() noexcept;
  bool exited() const noexcept { return exited_; }

  void post(base::weak_ref<Task>) noexcept;
  void post(duration_t delay, base::weak_ref<Task>) noexcept;

  static RunLoop& current() noexcept;

 private:
  struct TaskEntry;
  using run_queue_t = std::list<TaskEntry>;

  std::atomic<run_queue_t*> tasks_;
  bool exited_;
  bool running_;
};

}  // namespace base

#endif  // ANATIRRA_BASE_RUN_LOOP
