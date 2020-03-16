// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/ref_ptr.hpp"

#include "gmock/gmock-function-mocker.h"
#include "gmock/gmock-spec-builders.h"
#include "third-party/googletest/gmock.hpp"
#include "third-party/googletest/gtest.hpp"

class Object : public base::RefCounted {
 public:
  int data;
};

class ObjectWithDestroy : public base::RefCounted {
 public:
  int data;

  MOCK_METHOD(void, destroy, ());

  ~ObjectWithDestroy() noexcept { destroy(); }
};

class ObjectWithDestroyAndWeak : public Object, public base::WeakReferenced {
 public:
  MOCK_METHOD(void, destroy, ());

  ~ObjectWithDestroyAndWeak() noexcept { destroy(); }
};

TEST(RefPtrTest, NullByDefault) {
  base::ref_ptr<Object> ptr;

  EXPECT_FALSE(ptr);
  EXPECT_EQ(ptr, nullptr);
}

TEST(RefPtrTest, NonNullIfHoldValue) {
  auto ptr = base::make_ref_ptr<Object>();

  EXPECT_TRUE(ptr);
  EXPECT_NE(ptr, nullptr);
}

TEST(RefPtrTest, TwoPtrsFromOneRawAreEqual) {
  Object* object = new Object();

  auto ptr1 = base::ref_ptr(object);
  auto ptr2 = base::ref_ptr(object);

  EXPECT_EQ(ptr1, ptr2);
}

TEST(RefPtrTest, AssignNullptr) {
  auto ptr = base::make_ref_ptr<ObjectWithDestroy>();
  EXPECT_CALL(*ptr, destroy());
  ptr = nullptr;
  EXPECT_EQ(ptr, nullptr);
}

TEST(RefPtrTest, ConstructNullptr) {
  auto ptr = base::ref_ptr<Object>(nullptr);
  EXPECT_EQ(ptr, nullptr);
}

TEST(RefPtrTest, CopyMove) {
  auto ptr1 = base::make_ref_ptr<ObjectWithDestroy>();
  ptr1->data = 5;

  EXPECT_CALL(*ptr1, destroy());

  auto ptr2 = ptr1;
  auto ptr3 = std::move(ptr1);

  EXPECT_EQ(ptr1, nullptr);
  EXPECT_EQ(ptr2, ptr3);
  EXPECT_EQ(ptr2->data, 5);
}

TEST(RefPtrTest, AssignNullInstance) {
  auto p = base::make_ref_ptr<ObjectWithDestroy>();
  auto null = base::ref_ptr<ObjectWithDestroy>();

  EXPECT_CALL(*p, destroy());
  p = null;

  EXPECT_EQ(p, nullptr);
}

TEST(RefPtrTest, MoveNullInstance) {
  auto p = base::make_ref_ptr<ObjectWithDestroy>();
  auto null = base::ref_ptr<ObjectWithDestroy>();

  EXPECT_CALL(*p, destroy());
  p = std::move(null);

  EXPECT_EQ(p, nullptr);
}

TEST(RefPtrTest, MoveIntoNullInstance) {
  auto p0 = base::ref_ptr<ObjectWithDestroy>();
  auto p = base::make_ref_ptr<ObjectWithDestroy>();

  p0 = std::move(p);
  EXPECT_CALL(*p0, destroy());

  EXPECT_NE(p0, nullptr);
  EXPECT_EQ(p, nullptr);
}

TEST(RefPtrTest, MoveConstruct) {
  auto p = base::make_ref_ptr<ObjectWithDestroy>();
  p->data = 10;
  EXPECT_CALL(*p, destroy());

  auto p1 = base::ref_ptr(std::move(p));
  EXPECT_EQ(p1->data, 10);
  EXPECT_EQ(p, nullptr);
}

TEST(RefPtrTest, Reset) {
  auto p = base::make_ref_ptr<ObjectWithDestroy>();
  EXPECT_CALL(*p, destroy());

  p = base::make_ref_ptr<ObjectWithDestroy>();
  EXPECT_CALL(*p, destroy());
}

TEST(RefPtrTest, WeakSimple) {
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

TEST(RefPtrTest, WeakNullatesIfRefNullates) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_CALL(*p, destroy());
  EXPECT_TRUE(wp);
  EXPECT_NE(wp, nullptr);

  p = nullptr;

  EXPECT_FALSE(wp);
  EXPECT_EQ(wp, nullptr);
}

TEST(RefPtrTest, WeakNullatesIfRefReset) {
  auto p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();
  auto wp = base::weak_ref(p);
  EXPECT_CALL(*p, destroy());
  EXPECT_TRUE(wp);
  EXPECT_NE(wp, nullptr);

  p = base::make_ref_ptr<ObjectWithDestroyAndWeak>();

  EXPECT_FALSE(wp);
  EXPECT_EQ(wp, nullptr);
}

TEST(RefPtrTest, WeakNullatesIfRefDestroyed) {
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

TEST(RefPtrTest, WeakNullatesIfRefDestroyed2) {
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

TEST(RefPtrTest, MultipleWeakNullatesIfRefDestroyed) {
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

TEST(RefPtrTest, OtherWeaksAreIntactWhenOneIsDestroyed) {
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
