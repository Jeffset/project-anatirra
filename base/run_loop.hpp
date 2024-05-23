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

#pragma once

#include "base/macro.hpp"
#include "base/ref_ptr.hpp"
#include "base/weak_ref.hpp"

#include "base/config.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>

namespace base {

class BASE_PUBLIC RunLoop {
 public:
  DISABLE_COPY_MOVE(RunLoop);

  using clock_t = std::chrono::steady_clock;
  using duration_t = clock_t::duration;
  using time_point_t = clock_t::time_point;
  using task_t = std::function<void()>;

  struct Task {
    const task_t runnable;
  };

  struct DelayedTask : public base::WeakReferencedThreadSafe {
    const task_t runnable;
    const time_point_t time;
    explicit DelayedTask(task_t runnable, time_point_t time) noexcept 
      : runnable(runnable), time(time) {}
  };

  RunLoop() noexcept;
  ~RunLoop() noexcept;

  // Exits when no 
  void exit_when_idle() noexcept;

  bool exit_requested() const noexcept { return exit_when_idle_; }
  bool running() const noexcept { return running_; }

  void run(task_t on_idle);
  void post(task_t task) noexcept;
  NODISCARD base::ref_ptr<DelayedTask> post_delayed(task_t task, duration_t delay) noexcept;

  static RunLoop& current() noexcept;
  static RunLoop& main() noexcept;
  static void set_main(RunLoop* loop) noexcept;

 private:
  static constinit const auto kQueuesCount = 4;
  struct queue_t {
    std::vector<Task> tasks;
    std::vector<base::weak_ref<DelayedTask>> delayed_tasks;
  };

  static std::size_t queue_index() noexcept;
  queue_t* acquire_queue(std::size_t index) noexcept;
  void store_queue(queue_t*, std::size_t index) noexcept;

  std::array<queue_t, kQueuesCount> queues_;
  std::array<std::atomic<queue_t*>, kQueuesCount> queue_ptrs_;

  std::atomic<bool> exit_when_idle_;
  std::atomic<bool> running_;
  bool idle_;
};

}  // namespace base
