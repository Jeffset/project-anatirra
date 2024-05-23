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

#include "base/run_loop.hpp"

#include "base/debug/debug.hpp"
#include "base/ref_ptr.hpp"
#include "base/weak_ref.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <list>
#include <stack>
#include <thread>
#include <utility>
#include <vector>

namespace base {

namespace {
thread_local std::stack<RunLoop*, std::vector<RunLoop*>> tl_run_loops;
std::atomic<RunLoop*> g_main_loop;
}  // namespace

RunLoop::RunLoop() noexcept : 
  queues_{}
  , queue_ptrs_{}
  , exit_when_idle_(false)
  , running_(false)
  , idle_(false)
  {
    for (auto i = 0u; i < kQueuesCount; ++i) {
      queue_ptrs_[i] = &queues_[i];
    }
  }

RunLoop::~RunLoop() noexcept = default;

auto RunLoop::acquire_queue(std::size_t index) noexcept -> queue_t* {
  queue_t* queue;
  do {
    queue = queue_ptrs_[index].exchange(nullptr, std::memory_order::acquire);
  } while(!queue);
  return queue;
}

void RunLoop::store_queue(queue_t* queue, std::size_t index) noexcept {
  queue_ptrs_[index].store(queue, std::memory_order::release);
}

auto RunLoop::queue_index() noexcept -> std::size_t {
  const auto ti = std::this_thread::get_id();
  return std::hash<std::thread::id>{}(ti) % RunLoop::kQueuesCount;
}

void RunLoop::run(task_t on_idle) {
  ASSERT(!running_) << "Can't restart already running RunLoop";
  tl_run_loops.push(this);
  running_ = true;

  std::list<base::weak_ref<DelayedTask>> delayed_tasks;

  while (!exit_when_idle_ || !idle_) {
    if (idle_) {
      on_idle();
    }

    bool did_work = false;
    for (auto index = 0u; index < kQueuesCount; ++index) {
      auto* queue = acquire_queue(index);
      // Extract the queued tasks
      auto tasks = std::move(queue->tasks);
      auto new_delayed_tasks = std::move(queue->delayed_tasks);

      queue->tasks.clear();
      queue->delayed_tasks.clear();
      // Unlock the queue as fast as possible
      store_queue(queue, index);

      for (auto& task : tasks) {
        task.runnable();
      }

      for (auto&& delayed_task : new_delayed_tasks) {
        delayed_tasks.emplace_front(std::move(delayed_task));
      }

      did_work = did_work || !tasks.empty();
    }

    if (!delayed_tasks.empty()) {
      delayed_tasks.remove_if([&did_work](const base::weak_ref<DelayedTask>& task_ref) {
        if (auto task = task_ref.lock()) {
          if (task->time <= clock_t::now()) {
            task->runnable();
            did_work = true;
            return true;
          }

          return false;
        }

        return true;
      });
    }

    idle_ = !did_work;
  }
  ASSERT(tl_run_loops.top() == this) << "Invalid destruction order";
  tl_run_loops.pop();
  running_ = false;
}

void RunLoop::exit_when_idle() noexcept {
  exit_when_idle_ = true;
}

void RunLoop::post(task_t task) noexcept {
  const auto index = queue_index();
  auto* queue = acquire_queue(index);
  queue->tasks.emplace_back(std::move(task));
  store_queue(queue, index);
}

auto RunLoop::post_delayed(task_t task, duration_t delay) noexcept -> base::ref_ptr<DelayedTask> {
  auto delayed_task = base::make_ref_ptr<DelayedTask>(
      std::move(task), clock_t::now() + delay);
  const auto index = queue_index();
  auto* queue = acquire_queue(index);
  queue->delayed_tasks.emplace_back(delayed_task);
  store_queue(queue, index);
  return delayed_task;
}

// static
RunLoop& RunLoop::current() noexcept {
  ASSERT(!tl_run_loops.empty()) << "No current RunLoop";
  return *tl_run_loops.top();
}

// static
RunLoop& RunLoop::main() noexcept {
  auto main_loop = g_main_loop.load(std::memory_order::acquire);
  ASSERT(main_loop) << "No main RunLoop";
  return *main_loop;
}

// static
void RunLoop::set_main(RunLoop* main_loop) noexcept {
  auto* old = g_main_loop.exchange(main_loop, std::memory_order::release);
  ASSERT(!old) << "Main RunLoop already set";
}

}  // namespace base
