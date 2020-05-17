// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "cursedui/region.hpp"

#include "base/debug/debug.hpp"
#include "third-party/googletest/gmock.hpp"
#include "third-party/googletest/gtest.hpp"

#include <algorithm>
#include <vector>

using namespace cursedui::paint;
using namespace cursedui::gfx;

namespace {

Rect EMPTY_RECT = {0, 0, -1, 2};

std::vector<Rect> all_rects_from(const Region& region) {
  std::vector<Rect> result;
  for (auto& rect : region)
    result.push_back(rect);
  return result;
}

class LogInit {
 public:
  LogInit() { base::debug::setup_logging(&logger); }

 private:
  base::debug::LoggerToStdErr logger;
} g_log_init;

template <int W, int H>
class RegionTester {
 public:
  RegionTester() : matrix{} {}

  void paint(const std::vector<Rect> rects) {
    for (auto& r : rects)
      for (int x = r.left; x <= r.right; ++x)
        for (int y = r.top; y <= r.bottom; ++y)
          matrix[x][y]++;
  }
  void paint(const Region region) {
    for (auto& r : region)
      for (int x = r.left; x <= r.right; ++x)
        for (int y = r.top; y <= r.bottom; ++y)
          matrix[x][y]++;
  }

  void log() const {
    for (int y = 0; y < H; ++y) {
      std::stringstream oss{std::ios::in | std::ios::out};
      for (int x = 0; x < W; ++x)
        oss << matrix[x][y] << '.';
      LOG() << oss.rdbuf();
    }
  }

  bool has_overdraw() const {
    for (int x = 0; x < W; ++x)
      for (int y = 0; y < H; ++y)
        if (matrix[x][y] > 1)
          return true;
    return false;
  }

  bool operator==(const RegionTester& rhs) const {
    for (int x = 0; x < W; ++x)
      for (int y = 0; y < H; ++y)
        if (bool(matrix[x][y]) != bool(rhs.matrix[x][y]))
          return false;
    return true;
  }

 private:
  int matrix[W][H];
};

void test(const std::vector<Rect>& v) {
  using namespace cursedui::gfx::operators;
  Region region;
  RegionTester<22, 22> t1, t2;
  for (auto& r : v)
    region.add(r);

  t1.paint(v);
  t2.paint(region);

  t2.log();

  EXPECT_EQ(t1, t2);
  EXPECT_FALSE(t2.has_overdraw());

  for (auto& r1 : region)
    for (auto& r2 : region) {
      if (&r1 == &r2)
        continue;
      EXPECT_FALSE(r1.intersects(r2));
    }
}

}  // namespace

#define EXPECT_RECTS(region, ...) \
  EXPECT_EQ(all_rects_from(region), (std::vector{__VA_ARGS__}))

TEST(RegionTest, Initial) {
  Region region;

  EXPECT_TRUE(region.empty());
  EXPECT_TRUE(all_rects_from(region).empty());
}

TEST(RegionTest, EmptyIfAddEmptyRects) {
  ASSERT_FALSE(EMPTY_RECT.has_area());

  Region region;
  region.add(EMPTY_RECT);
  region.add(EMPTY_RECT);

  region.add(EMPTY_RECT);
  EXPECT_TRUE(region.empty());
}

TEST(RegionTest, AddNonEmptyRect) {
  Region region;

  region.add(EMPTY_RECT);
  region.add(Rect{});
  region.add(EMPTY_RECT);

  EXPECT_FALSE(region.empty());
  EXPECT_RECTS(region, Rect{});
}

TEST(RegionTest, AddSubRect) {
  Region region;

  region.add(Rect{0, 0, 1, 1});
  region.add(Rect{0, 0, 0, 0});

  EXPECT_RECTS(region, Rect{0, 0, 1, 1});
}

TEST(RegionTest, AddSuperRect) {
  Region region;

  region.add(Rect{10, 10, 20, 20});
  region.add(Rect{0, 0, 20, 20});

  EXPECT_RECTS(region, Rect{0, 0, 20, 20});
}

TEST(RegionTest, SimpleSequence) {
  test({
      Rect{0, 0, 5, 10},
      Rect{2, 4, 10, 20},
  });
}

TEST(RegionTest, SimpleSequenceBottomLeft) {
  test({
      Rect{5, 0, 10, 5},
      Rect{0, 5, 5, 20},
  });
}

TEST(RegionTest, SimpleSequenceTopRight) {
  test({
      Rect{0, 10, 5, 20},
      Rect{5, 0, 15, 10},
  });
}

TEST(RegionTest, SimpleSequenceReversed) {
  test({
      Rect{2, 4, 10, 20},
      Rect{0, 0, 5, 10},
  });
}

TEST(RegionTest, NormalSequence) {
  test({
      Rect{1, 1, 2, 2},
      Rect{3, 3, 4, 4},
      Rect{2, 2, 3, 6},
      Rect{2, 4, 6, 5},
  });
}

TEST(RegionTest, LargeSequence) {
  test({
      Rect{0, 0, 20, 2},
      Rect{3, 3, 4, 4},
      Rect{5, 0, 10, 5},
      Rect{2, 7, 18, 8},
      Rect{5, 5, 5, 20},
      Rect{2, 4, 6, 5},
  });
}
