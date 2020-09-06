// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/run_loop.hpp"

#include "base/debug/debug.hpp"
#include "third-party/googletest/gmock.hpp"
#include "third-party/googletest/gtest.hpp"

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
      loop.exit();
    }
    dummy.act();
  });
}

TEST_F(RunLoopTest, CurrentIsSetCurrectly) {
  base::RunLoop loop;

  loop.run([&loop]() {
    base::RunLoop nested;
    ASSERT_EQ(&base::RunLoop::current(), &loop);
    base::RunLoop::current().exit();

    nested.run([&nested]() {
      ASSERT_EQ(&base::RunLoop::current(), &nested);
      base::RunLoop::current().exit();
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
  std::vector<base::ref_ptr<base::RunLoop::Task>> tasks(THREAD_COUNT * POST_COUNT);
  posters.reserve(THREAD_COUNT);
  for (int i = 0; i < THREAD_COUNT; ++i) {
    for (int j = 0; j < POST_COUNT; ++j) {
      auto payload = (i << (sizeof(short) * 8)) | j;
      EXPECT_CALL(dummy, act1(payload)).Times(2);
    }
    std::thread poster([&, i]() {
      while (!go)  // this is done to increase contention.
        ;
      for (int j = 0; j < POST_COUNT; ++j) {
        auto t = t1_loop.post([&dummy, i, j]() {
          auto payload = (i << (sizeof(short) * 8)) | j;
          dummy.act1(payload);
        });
        t2_loop.repost(t);
        tasks[i * POST_COUNT + j] = t;
        std::this_thread::yield();
      }
    });
    posters.emplace_back(std::move(poster));
  }
  go = true;
  for (auto& t : posters) {
    t.join();
  }

  auto exiter = t1_loop.post([]() { base::RunLoop::current().exit(); });
  t2_loop.repost(exiter);
  t1.join();
  t2.join();
}
