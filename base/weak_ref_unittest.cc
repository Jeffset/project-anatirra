// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/weak_ref.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

class ObjectWithDestroy : public base::RefCounted {
 public:
  int data;

  MOCK_METHOD(void, destroy, ());

  ~ObjectWithDestroy() noexcept { destroy(); }
};

class ObjectWithDestroyAndWeak : public base::WeakReferenced {
 public:
  int data;

  MOCK_METHOD(void, destroy, ());

  virtual ~ObjectWithDestroyAndWeak() noexcept { destroy(); }
};

class Object : public base::WeakReferenced {};

class DerivedObject : public Object {
 public:
  ~DerivedObject() override = default;
};

}  // namespace

TEST(WeakRefTest, WeakSimple) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  p->data = 5;
  EXPECT_CALL(*p, destroy());
  {
    auto wp = base::weak_ref(p);
    wp.lock()->data = 5;
  }
  {
    auto wp = base::weak_ref(p);
    wp.lock()->data = 10;
  }
  EXPECT_EQ(p->data, 10);
}

TEST(WeakRefTest, WeakNullatesIfRefNullates) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_CALL(*p, destroy());
  EXPECT_TRUE(wp.lock());
  EXPECT_NE(wp, nullptr);

  p = nullptr;

  EXPECT_FALSE(wp.lock());
  EXPECT_EQ(wp, nullptr);
}

TEST(WeakRefTest, WeakNullatesIfRefReset) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_CALL(*p, destroy());
  EXPECT_TRUE(wp.lock());
  EXPECT_NE(wp, nullptr);

  p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();

  EXPECT_FALSE(wp.lock());
  EXPECT_EQ(wp, nullptr);
}

TEST(WeakRefTest, WeakNullatesIfRefDestroyed) {
  auto wp = base::weak_ref<ObjectWithDestroyAndWeak>();
  EXPECT_FALSE(wp.lock());
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    auto tmp = base::weak_ref(p);
    wp = std::move(tmp);
    EXPECT_TRUE(wp.lock());
  }
  EXPECT_FALSE(wp.lock());
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    auto tmp = base::weak_ref(p);
    wp = std::move(tmp);
    EXPECT_TRUE(wp.lock());
  }
  EXPECT_FALSE(wp.lock());
}

TEST(WeakRefTest, WeakNullatesIfRefDestroyed2) {
  auto wp = base::weak_ref<ObjectWithDestroyAndWeak>();
  EXPECT_FALSE(wp.lock());
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    wp = p;
    EXPECT_TRUE(wp.lock());
  }
  EXPECT_FALSE(wp.lock());
  {
    auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
    EXPECT_CALL(*p, destroy());
    wp = p;
    EXPECT_TRUE(wp.lock());
  }
  EXPECT_FALSE(wp.lock());
}

TEST(WeakRefTest, MultipleWeakNullatesIfRefDestroyed) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  auto wp1 = base::weak_ref(p);
  auto wp2 = wp1;
  auto wp3 = std::move(wp1);
  base::weak_ref<ObjectWithDestroyAndWeak> wp4;
  wp4 = wp3;

  EXPECT_EQ(wp, wp2);
  EXPECT_EQ(wp, wp3);
  EXPECT_EQ(wp, wp4);
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
  EXPECT_TRUE(wp.lock());

  {
    auto wp1 = base::weak_ref(p);
    EXPECT_EQ(wp1.lock(), p);
  }

  {
    auto wp2 = wp;
    EXPECT_EQ(wp2.lock(), p);
  }

  EXPECT_EQ(wp.lock(), p);
}

TEST(WeakRefTest, DerivedTest) {
  base::weak_ref<Object> wp0;
  auto rp = base::make_ref_ptr<DerivedObject>();
  base::weak_ref<Object> wp = rp;
  wp0 = rp;
  auto wp2 = wp;
  auto p = rp;
  wp = p;
  wp0 = wp;
}

TEST(WeakRefTest, ConstTest) {
  auto rp = base::make_ref_ptr<const Object>();
  auto wp = base::weak_ref(rp);
}
