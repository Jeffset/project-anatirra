// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/run_loop.hpp"

#include "base/debug/debug.hpp"

#include <chrono>
#include <forward_list>
#include <memory>
#include <stack>
#include <vector>

namespace base {

namespace {
thread_local std::stack<RunLoop*, std::vector<RunLoop*>> g_run_loops;
}  // namespace

struct RunLoop::TaskEntry {
  base::weak_ref<Task> task;
  time_point_t time;
};

RunLoop::RunLoop() noexcept : tasks_(nullptr), exited_(false), running_(false) {}

RunLoop::~RunLoop() noexcept {}

void RunLoop::run(run_t loop) {
  ASSERT(!running_) << "Can't restart already running RunLoop";
  ASSERT(!exited_) << "Can't restart exited RunLoop";
  g_run_loops.push(this);
  running_ = true;

  std::forward_list<TaskEntry> delayed_tasks;

  while (!exited_) {
    loop();
    auto now = clock_t::now();
    if (auto queued_tasks = std::unique_ptr<run_queue_t>(
            tasks_.exchange(nullptr, std::memory_order_acquire))) {
      for (auto& task_entry : *queued_tasks) {
        if (auto task = task_entry.task.lock()) {
          ASSERT(task->runnable) << "Can't invoke empty function";
          if (task_entry.time > now) {
            // If task has a scheduled moment when to run - delay it.
            delayed_tasks.emplace_front(std::move(task_entry));
          } else {
            task->runnable();
            now = clock_t::now();
          }
        }
      }
    }
    delayed_tasks.remove_if([&now](const TaskEntry& task_entry) -> bool {
      auto task = task_entry.task.lock();

      if (!task)
        return true;

      if (task_entry.time <= now) {
        task->runnable();
        now = clock_t::now();
        return true;
      }

      return false;
    });
  }
  ASSERT(g_run_loops.top() == this) << "Invalid destruction order";
  g_run_loops.pop();
  running_ = false;
}

void RunLoop::exit() noexcept {
  exited_ = true;
}

void RunLoop::post(duration_t delay, base::weak_ref<Task> task) noexcept {
  std::unique_ptr<run_queue_t> tasks;
  const TaskEntry entry{task, clock_t::now() + delay};
  while (true) {
    // acquire ("lock") and take ownership for currently posted tasks.
    auto acquired =
        std::unique_ptr<run_queue_t>(tasks_.exchange(nullptr, std::memory_order_acquire));
    if (!tasks) {
      // First iteration
      if (acquired) {
        // add task into the preexisting queue.
        tasks = std::move(acquired);
      } else {
        // no tasks were queued, so create new queue with our new task.
        tasks = std::make_unique<run_queue_t>();
      }
      tasks->push_back(entry);
    } else {
      // Add before if anything.
      if (acquired) {
        tasks->insert(tasks->end(), acquired->begin(), acquired->end());
      }
    }

    // try to set a queue back, if it hasn't changed.
    run_queue_t* expected = nullptr;
    if (tasks_.compare_exchange_strong(expected, tasks.get(), std::memory_order_release,
                                       std::memory_order_relaxed)) {
      // successfully changed: release ownership and return.
      tasks.release();
      return;
    }
    // if failed - retry.
  }
}

void RunLoop::post(base::weak_ref<Task> task) noexcept {
  post(duration_t{}, std::move(task));
}

// static
RunLoop& RunLoop::current() noexcept {
  ASSERT(!g_run_loops.empty()) << "No current RunLoop";
  return *g_run_loops.top();
}

}  // namespace base
