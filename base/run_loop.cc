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

// TODO: move this into base?

template <class T, class... Args>
auto pmr_wrap_unique(std::pmr::memory_resource* memory, T* ptr) {
  const auto deleter = [memory](T* ptr) {
    if (ptr == nullptr)
      return;
    std::pmr::polymorphic_allocator<T> alloc{memory};
    std::allocator_traits<decltype(alloc)>::destroy(alloc, ptr);
    alloc.deallocate(ptr, 1);
  };
  return std::unique_ptr<T, decltype(deleter)>(ptr, deleter);
}

template <class T, class... Args>
auto pmr_make_unique(std::pmr::memory_resource* memory, Args&&... args) {
  void* mem = memory->allocate(sizeof(T), alignof(T));
  T* ptr = new (mem) T(std::forward<Args>(args)...);
  return pmr_wrap_unique(memory, ptr);
}

template <class T, class... Args>
auto pmr_make(std::pmr::memory_resource* memory, Args&&... args) {
  std::pmr::polymorphic_allocator<T> alloc{memory};
  T* ptr = alloc.allocate(1);
  alloc.construct(ptr, std::forward<Args>(args)...);
  return ptr;
}
}  // namespace

struct RunLoop::TaskNode {
  base::weak_ref<Task> task;
  time_point_t time;
  TaskNode* next;

  TaskNode(base::weak_ref<Task> task, time_point_t time)
      : task(std::move(task)), time(time), next(nullptr) {}

  struct iterator {
    using difference_type = int;
    using value_type = TaskNode;
    using pointer = TaskNode*;
    using reference = TaskNode&;
    using iterator_category = std::forward_iterator_tag;

    iterator& operator++() noexcept {
      ASSERT(current_);
      current_ = current_->next;
      return *this;
    }
    bool operator!=(const iterator& rhs) const noexcept {
      return current_ != rhs.current_;
    }
    TaskNode& operator*() noexcept { return *current_; }

    TaskNode* current_ = nullptr;
  };

  iterator begin() noexcept { return {this}; }
  iterator end() const noexcept { return {}; }
};

RunLoop::RunLoop() noexcept : tasks_(nullptr), exited_(false), running_(false) {}

RunLoop::~RunLoop() noexcept {}

void RunLoop::run_impl(Task& loop) {
  ASSERT(!running_) << "Can't restart already running RunLoop";
  ASSERT(!exited_) << "Can't restart exited RunLoop";
  g_run_loops.push(this);
  running_ = true;

  std::pmr::unsynchronized_pool_resource memory;
  std::pmr::forward_list<TaskNode> delayed_tasks{&memory};

  while (!exited_) {
    loop.run();
    auto now = clock_t::now();
    if (auto queued_tasks = pmr_wrap_unique(
            &memory_pool_, tasks_.exchange(nullptr, std::memory_order_acquire))) {
      // FIXME(jeffset): reverse list.
      for (auto& task_entry : *queued_tasks) {
        if (auto task = task_entry.task.lock()) {
          if (task_entry.time > now) {
            // If task has a scheduled moment when to run - delay it.
            delayed_tasks.emplace_front(std::move(task_entry));
          } else {
            task->run();
            now = clock_t::now();
          }
        }
      }
    }
    delayed_tasks.remove_if([&now](const TaskNode& task_entry) -> bool {
      auto task = task_entry.task.lock();

      if (!task)
        return true;

      if (task_entry.time <= now) {
        task->run();
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

void RunLoop::post_impl(duration_t delay, base::weak_ref<Task> task) noexcept {
  TaskNode* last = pmr_make<TaskNode>(&memory_pool_, task, clock_t::now() + delay);
  TaskNode* first = last;
  while (true) {
    // acquire ("lock") and take ownership for currently posted tasks.
    auto* acq = tasks_.exchange(nullptr, std::memory_order_acquire);
    if (acq) {
      last = last->next = acq;
      while (last->next) {
        last = last->next;
      }
    }

    // try to set a queue back, if it hasn't changed.
    TaskNode* expected = nullptr;
    if (tasks_.compare_exchange_strong(expected, first, std::memory_order_release,
                                       std::memory_order_relaxed)) {
      // successfully changed: release ownership and return.
      return;
    }
    // if failed - retry.
  }
}

// static
RunLoop& RunLoop::current() noexcept {
  ASSERT(!g_run_loops.empty()) << "No current RunLoop";
  return *g_run_loops.top();
}

}  // namespace base
