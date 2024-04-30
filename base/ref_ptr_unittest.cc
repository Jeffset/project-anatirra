// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "base/ref_ptr.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

class Object : public base::RefCounted {
 public:
  int data;

  void action() {}
  void action_const() const {}
};

class Object2 : public base::RefCounted {};

class DerivedObject : public Object {};

class ObjectWithDestroy : public base::RefCounted {
 public:
  int data;

  MOCK_METHOD(void, destroy, ());

  ~ObjectWithDestroy() noexcept { destroy(); }
};

}  // namespace

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

TEST(RefPtrTest, ConstTest) {
  auto p = base::make_ref_ptr<Object>();
  p->action();
  p->action_const();

  const auto cp = base::make_ref_ptr<Object>();
  cp->action();
  cp->action_const();

  const auto pc = base::make_ref_ptr<const Object>();
  //  pc->action();
  pc->action_const();
}

TEST(RefPtrTest, DerivedTest) {
  auto d1 = base::make_ref_ptr<DerivedObject>();
  auto dd = base::make_ref_ptr<Object2>();
  base::ref_ptr<Object> p1{base::make_ref_ptr<DerivedObject>()};
  base::ref_ptr<Object> p2{d1};
  p1 = base::make_ref_ptr<DerivedObject>();
  p1 = d1;
  p1 = std::move(d1);
}
