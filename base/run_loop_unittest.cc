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
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <thread>
#include <vector>

namespace {

class Dummy {
 public:
  MOCK_METHOD(void, act, ());
  MOCK_METHOD(void, act1, (int));
};

}  // namespace

class RunLoopTest : public testing::Test {
  void SetUp() override { base::debug::setup_logging(&logger_); }
  base::debug::LoggerToStdErr logger_;
};

TEST_F(RunLoopTest, ExitWorks) {
  base::RunLoop loop;
  Dummy dummy;

  EXPECT_CALL(dummy, act).Times(20);

  int i = 0;
  loop.run([&]() {
    if (++i == 20) {
      loop.exit_when_idle();
    }
    dummy.act();
  });
}

TEST_F(RunLoopTest, CurrentIsSetCurrectly) {
  base::RunLoop loop;

  loop.run([&loop]() {
    base::RunLoop nested;
    ASSERT_EQ(&base::RunLoop::current(), &loop);
    base::RunLoop::current().exit_when_idle();

    nested.run([&nested]() {
      ASSERT_EQ(&base::RunLoop::current(), &nested);
      base::RunLoop::current().exit_when_idle();
    });

    ASSERT_EQ(&base::RunLoop::current(), &loop);
  });
}

TEST_F(RunLoopTest, MultiThread) {
  constexpr short THREAD_COUNT = 64;
  constexpr short POST_COUNT = 16;
  Dummy dummy;

  base::RunLoop t1_loop{}, t2_loop{};
  std::thread t1([&t1_loop]() { t1_loop.run([]() { std::this_thread::yield(); }); });
  std::thread t2([&t2_loop]() { t2_loop.run([]() { std::this_thread::yield(); }); });

  volatile bool go = false;
  std::vector<std::thread> posters;
  posters.reserve(THREAD_COUNT);
  for (int i = 0; i < THREAD_COUNT; ++i) {
    for (int j = 0; j < POST_COUNT; ++j) {
      auto payload = (i << (sizeof(short) * 8)) | j;
      EXPECT_CALL(dummy, act1(payload)).Times(2);
    }
  }
  for (int i = 0; i < THREAD_COUNT; ++i) {
    std::thread poster([&, i]() {
      while (!go)  // this is done to increase contention.
        ;
      for (int j = 0; j < POST_COUNT; ++j) {
        auto t = [&dummy, i, j]() {
          auto payload = (i << (sizeof(short) * 8)) | j;
          dummy.act1(payload);
        };
        t1_loop.post(t);
        t2_loop.post(t);
        std::this_thread::yield();
      }
    });
    posters.emplace_back(std::move(poster));
  }
  go = true;
  for (auto& t : posters) {
    t.join();
  }

  t1_loop.exit_when_idle();
  t2_loop.exit_when_idle();
  t1.join();
  t2.join();
}

TEST_F(RunLoopTest, SequentialConsistency) {
  constexpr short THREAD_COUNT = 50;
  constexpr short POST_COUNT = 16;

  std::map<int, std::vector<int>> calls;

  base::RunLoop loop{};

  volatile bool go = false;
  std::vector<std::thread> posters;
  posters.reserve(THREAD_COUNT);
  for (int i = 0; i < THREAD_COUNT; ++i) {
    std::thread poster([&, i]() {
      while (!go)  // this is done to increase contention.
        ;
      for (int j = 0; j < POST_COUNT; ++j) {
        auto t = [&calls, i, j]() {
          calls[i].push_back(j);
        };
        loop.post(t);
        std::this_thread::yield();
      }
    });
    posters.emplace_back(std::move(poster));
  }
  go = true;
  for (auto& t : posters) {
    t.join();
  }

  loop.exit_when_idle();

  loop.run([]{});

  for (const auto& [tid, nums] : calls) {
    int current = -1;
    for (int num : nums) {
      EXPECT_GE(num, current) << "For tid = " << tid;
      current = num;
    }
  }
}
