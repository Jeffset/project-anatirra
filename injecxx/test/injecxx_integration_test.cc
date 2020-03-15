// Copyright (C) 2020 Marco Jeffset (f.giffist@yandex.ru)
// This software is a part of the Anatirra Project.
// "Nothing is certain, but we shall hope."

#include "injecxx/injecxx_context.hpp"
#include "third-party/googletest/gmock.hpp"
#include "third-party/googletest/gtest.hpp"

#include <variant>

using namespace injecxx;

class Recorder {
 public:
  Recorder() noexcept = default;

  MOCK_METHOD(void, created, (int));
  MOCK_METHOD(void, destroyed, (int));

  MOCK_METHOD(void, act, (int));

  MAKE_FULLY_STATIONAR(Recorder);
};

class RecordableComponent {
 protected:
  ~RecordableComponent() = default;

 public:
  RecordableComponent(Recorder& recorder) noexcept : recorder(recorder) {}

  MAKE_FULLY_STATIONAR(RecordableComponent);

 protected:
  Recorder& recorder;
};

template <int N>
class CreateDestroyRecordable : public RecordableComponent {
 public:
  CreateDestroyRecordable(Recorder& r) noexcept : RecordableComponent(r) {
    recorder.created(N);
  }

 protected:
  ~CreateDestroyRecordable() { recorder.destroyed(N); }
};

class InjecxxTest : public testing::Test {
 protected:
  class Com1 : public CreateDestroyRecordable<1> {
   public:
    virtual ~Com1() = default;

    Com1(Recorder& r) noexcept : CreateDestroyRecordable(r) {}

    virtual void act(){};
  };

  class Com3;

  class Com2 : public CreateDestroyRecordable<2> {
   public:
    Com2(Recorder& r, Com1& com1, lazy<Com3> com3) noexcept
        : CreateDestroyRecordable(r), com1(com1), com3(com3) {}
    ~Com2();

    void act() { com1.act(); }

    Com1& com1;
    lazy<Com3> com3;
  };

  class Com3 : public CreateDestroyRecordable<3> {
   public:
    Com3(Recorder& r, Com1& com1) noexcept : CreateDestroyRecordable(r), com1(com1) {}

    Com1& com1;
  };

  class Com4 : public CreateDestroyRecordable<4> {
   public:
    Com4(Recorder& r, Com1&) noexcept : CreateDestroyRecordable(r) {}
  };

  class SubcontextCom : public CreateDestroyRecordable<5> {
   public:
    SubcontextCom(Recorder& r) noexcept : CreateDestroyRecordable(r) {}

    template <class ParentContext>
    void subcontext_lifecycle(ParentContext& c) {
      auto local = make_context<Com2, Com4>(c);
      local.dispatch([](Com2&) {});
      local.dispatch([](Com4&) {});
    }
  };
};

InjecxxTest::Com2::~Com2() {
  com3.get();
}

TEST_F(InjecxxTest, CreateDestroyOrder) {
  auto ctx = make_context<Recorder, Com1, Com2, Com3, Com4>();

  ctx.dispatch([](Recorder& rec) {
    EXPECT_CALL(rec, created(4)).Times(0);
    EXPECT_CALL(rec, created(4)).Times(0);

    testing::InSequence seq;
    EXPECT_CALL(rec, created(1));
    EXPECT_CALL(rec, created(2));
    EXPECT_CALL(rec, created(3));

    EXPECT_CALL(rec, destroyed(2));
    EXPECT_CALL(rec, destroyed(3));
    EXPECT_CALL(rec, destroyed(1));
  });

  ctx.dispatch([](Com2&) {});
}

TEST_F(InjecxxTest, ContextHierarchyTall) {
  auto ctx0 = make_context<Recorder>();
  auto ctx1 = make_context<Com1>(ctx0);
  auto ctx2 = make_context<Com3>(ctx1);
  auto ctx3 = make_context<Com2>(ctx2);
  auto ctx4 = make_context<Com4>(ctx3);

  ctx0.dispatch([](Recorder& rec) {
    EXPECT_CALL(rec, created(4)).Times(0);
    EXPECT_CALL(rec, created(4)).Times(0);

    testing::InSequence seq;
    EXPECT_CALL(rec, created(1));
    EXPECT_CALL(rec, created(2));
    EXPECT_CALL(rec, created(3));

    EXPECT_CALL(rec, destroyed(2));
    EXPECT_CALL(rec, destroyed(3));
    EXPECT_CALL(rec, destroyed(1));
  });

  ctx3.dispatch([](Com2&) {});
}

TEST_F(InjecxxTest, ContextHierarchyFat) {
  auto ctx0 = make_context<Recorder, Com1, Com3>();
  auto ctx1 = make_context<Com2, Com4>(ctx0);

  ctx0.dispatch([](Recorder& rec) {
    EXPECT_CALL(rec, created(4)).Times(0);
    EXPECT_CALL(rec, created(4)).Times(0);

    testing::InSequence seq;
    EXPECT_CALL(rec, created(1));
    EXPECT_CALL(rec, created(2));
    EXPECT_CALL(rec, created(3));

    EXPECT_CALL(rec, destroyed(2));
    EXPECT_CALL(rec, destroyed(3));
    EXPECT_CALL(rec, destroyed(1));
  });

  ctx1.dispatch([](Com2&) {});
}

TEST_F(InjecxxTest, ContextHierarchyTree) {
  auto ctx0 = make_context<Recorder, Com1>();
  auto ctx01 = make_context<Com2, Com3>(ctx0);
  auto ctx02 = make_context<Com4>(ctx0);

  ctx0.dispatch([](Recorder& rec) {
    testing::InSequence seq;
    EXPECT_CALL(rec, created(1));
    EXPECT_CALL(rec, created(2));
    EXPECT_CALL(rec, created(4));

    EXPECT_CALL(rec, destroyed(4));

    EXPECT_CALL(rec, created(3));

    EXPECT_CALL(rec, destroyed(2));
    EXPECT_CALL(rec, destroyed(3));
    EXPECT_CALL(rec, destroyed(1));
  });

  ctx01.dispatch([](Com2&) {});
  ctx02.dispatch([](Com4&) {});
}

TEST_F(InjecxxTest, DispatchSubcontext) {
  auto ctx0 = make_context<Recorder, Com1, Com3, SubcontextCom>();

  ctx0.dispatch([](Recorder& rec) {
    testing::InSequence seq;
    EXPECT_CALL(rec, created(5));

    // in local context
    EXPECT_CALL(rec, created(1));
    EXPECT_CALL(rec, created(2));
    EXPECT_CALL(rec, created(4));

    // destroy seq of local
    EXPECT_CALL(rec, destroyed(4));
    EXPECT_CALL(rec, created(3));
    EXPECT_CALL(rec, destroyed(2));

    // destroy seq of parent
    EXPECT_CALL(rec, destroyed(5));
    EXPECT_CALL(rec, destroyed(3));
    EXPECT_CALL(rec, destroyed(1));
  });

  ctx0.dispatch(
      [](SubcontextCom& subcom, auto& parent) { subcom.subcontext_lifecycle(parent); });
}

TEST_F(InjecxxTest, PolymorphDependency) {
  class Com1Impl1 : public Com1 {
   public:
    Com1Impl1(Recorder& r) noexcept : Com1(r) {}

    void act() override { recorder.act(1); }
  };

  class Com1Impl2 : public Com1 {
   public:
    Com1Impl2(Recorder& r) noexcept : Com1(r) {}

    void act() override { recorder.act(2); }
  };

  auto ctx_r = make_context<Recorder>();

  auto ctx01 = make_context<Com1Impl1, Com3>(ctx_r);
  auto ctx1 = make_context<Com2, Com4>(ctx01);

  auto ctx02 = make_context<Com1Impl2, Com3>(ctx_r);
  auto ctx2 = make_context<Com2, Com4>(ctx02);

  ctx_r.dispatch([](Recorder& r) {
    testing::InSequence seq;
    EXPECT_CALL(r, act(2));
    EXPECT_CALL(r, act(1));
  });

  auto dispatcher = [](Com2& com2) { com2.act(); };
  ctx2.dispatch(dispatcher);
  ctx1.dispatch(dispatcher);
}
