// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/weak_ref.hpp"

#include "gmock/gmock-function-mocker.h"
#include "gmock/gmock-spec-builders.h"
#include "third-party/googletest/gmock.hpp"
#include "third-party/googletest/gtest.hpp"

class ObjectWithDestroy : public base::RefCounted {
 public:
  int data;

  MOCK_METHOD(void, destroy, ());

  ~ObjectWithDestroy() noexcept { destroy(); }
};

class ObjectWithDestroyAndWeak : public ObjectWithDestroy, public base::WeakReferenced {
 public:
  MOCK_METHOD(void, destroy, ());

  ~ObjectWithDestroyAndWeak() noexcept { destroy(); }
};

TEST(WeakRefTest, WeakSimple) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  p->data = 5;
  EXPECT_CALL(*p, destroy());
  {
    auto wp = base::weak_ref(p);
    wp.get()->data = 5;
  }
  {
    auto wp = base::weak_ref(p);
    wp.get()->data = 10;
  }
  EXPECT_EQ(p->data, 10);
}

TEST(WeakRefTest, WeakNullatesIfRefNullates) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_CALL(*p, destroy());
  EXPECT_TRUE(wp);
  EXPECT_NE(wp, nullptr);

  p = nullptr;

  EXPECT_FALSE(wp);
  EXPECT_EQ(wp, nullptr);
}

TEST(WeakRefTest, WeakNullatesIfRefReset) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_CALL(*p, destroy());
  EXPECT_TRUE(wp);
  EXPECT_NE(wp, nullptr);

  p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();

  EXPECT_FALSE(wp);
  EXPECT_EQ(wp, nullptr);
}

TEST(WeakRefTest, WeakNullatesIfRefDestroyed) {
  auto wp = base::weak_ref<ObjectWithDestroyAndWeak>();
  EXPECT_FALSE(wp);
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    auto tmp = base::weak_ref(p);
    wp = std::move(tmp);
    EXPECT_TRUE(wp);
  }
  EXPECT_FALSE(wp);
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    auto tmp = base::weak_ref(p);
    wp = std::move(tmp);
    EXPECT_TRUE(wp);
  }
  EXPECT_FALSE(wp);
}

TEST(WeakRefTest, WeakNullatesIfRefDestroyed2) {
  auto wp = base::weak_ref<ObjectWithDestroyAndWeak>();
  EXPECT_FALSE(wp);
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    wp = p;
    EXPECT_TRUE(wp);
  }
  EXPECT_FALSE(wp);
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    wp = p;
    EXPECT_TRUE(wp);
  }
  EXPECT_FALSE(wp);
}

TEST(WeakRefTest, MultipleWeakNullatesIfRefDestroyed) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  auto wp1 = base::weak_ref(p);
  auto wp2 = wp1;
  auto wp3 = std::move(wp1);

  EXPECT_EQ(wp, wp2);
  EXPECT_EQ(wp, wp3);
  EXPECT_EQ(wp1, nullptr);

  EXPECT_CALL(*p, destroy());
  p = nullptr;

  EXPECT_EQ(wp, nullptr);
  EXPECT_EQ(wp2, nullptr);
  EXPECT_EQ(wp3, nullptr);
}

TEST(WeakRefTest, OtherWeaksAreIntactWhenOneIsDestroyed) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_TRUE(wp);

  {
    auto wp1 = base::weak_ref(p);
    EXPECT_EQ(wp1.get_ref_ptr(), p);
  }

  {
    auto wp2 = wp;
    EXPECT_EQ(wp2.get_ref_ptr(), p);
  }

  EXPECT_EQ(wp.get_ref_ptr(), p);
}
