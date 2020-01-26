// COMPILER TEST EXPECTS: FAILS

#include "base/type_array.hpp"
#include "injecxx/injecxx.hpp"
#include "injecxx/injecxx_context.hpp"

#include <iostream>
#include <type_traits>

using namespace base;
using namespace base::injecxx;

class Foo;

class ComG {
 public:
  explicit ComG(Foo&) noexcept : f(4) {}

  const char* d = "comG";

  const int f;

  MAKE_FULLY_STATIONAR(ComG);
};

class Com1 {
 public:
  Com1() = default;

  const char* d = "com1";

  MAKE_FULLY_STATIONAR(Com1);
};

class Com0 {
 public:
  Com0(lazy<Com1> c1) noexcept : c1_(c1) {}

  const char* d = "C0";

  void p() { std::cout << "C0: " << c1_.get().d << '\n'; }

  MAKE_FULLY_STATIONAR(Com0);

 private:
  lazy<Com1> c1_;
};

class Com2 {
 public:
  Com2(Com1&, Com0& c0) noexcept {
    std::cout << "Com2 constructed\n";
    c0.p();
  }

  void act() { std::cout << "Com2 act\n"; }

  MAKE_FULLY_STATIONAR(Com2);
};

class Foo {
 public:
  Foo(Com1&, Com2& c2) noexcept : c2_(c2) { std::cout << "Foo constructed\n"; }

  Com2& c2_;

  MAKE_FULLY_STATIONAR(Foo);
};

template <template <typename> typename P>
constexpr auto filter_expr() {
  return [](auto t) { return P<meta::ta_single_t<decltype(t)>>::value; };
}

template <class... Ts1, class... Ts2>
void summ(meta::ta<Ts2...>) {
  static_assert(sizeof...(Ts2) == 1);
}

class A {
 public:
  virtual void act() { std::cout << "A acting" << g << "\n"; }
  int g = 0;
};

class B : public A {
 public:
  B() { g = 1; }
  void act() override { std::cout << "B acting\n"; }
};

template <class Filter, class... Args>
void print_smart(Filter filter, Args... args) {
  constexpr auto types = meta::ta<Args...>{};
  constexpr auto filtered = meta::filter(types, filter);
  if constexpr (filtered != meta::empty_ta) {
    meta::for_each(filtered, [](auto) { std::cout << "*"; });
  } else {
    std::cout << "&";
  }
  (std::cout << ... << args) << '\n';
}

auto gen_filt(bool) {
  return [](auto) { return false; };
}

template <class Object, class... Ts>
void dispatch(void (Object::*func)(), Ts... ts) {
  constexpr auto tts = meta::ta<Ts...>{};
  (
      [&ts, &func]() {
        if constexpr (std::is_same_v<Object, Ts>) {
          (ts->func)();
        }
      }(),
      ...);
}

template <class Context>
void consume_foo(Foo& foo, Context& context) {
  auto ctx = injecxx::make_context<>(context);
  foo.c2_.act();
}

int main() {
  using namespace meta;

  A a;
  a.act();
  B b;
  a = b;
  a.act();

  auto filt = gen_filt(false);
  print_smart(filt, 1, 2, 3);

  summ<int, double, float>(t<int>);

  auto ctx_1 = injecxx::make_context<Com1, Com2, Com0>();
  auto ctx = injecxx::make_context<ComG, Foo>(ctx_1);

  ctx.dispatch(meta::is_type<Foo>(),
               [](Foo& instance, auto& context) { consume_foo(instance, context); });
  ctx.dispatch([](Foo& instance, auto& context) { consume_foo(instance, context); });

  //  static_assert(noexcept(ctx.get<Foo>()));
  //  static_assert(noexcept(ctx.~context()));

  //  constexpr auto quf =
  //      deduce_constructor_arg_types(ta<Foo>(), ta<Com2, Com1, Com0>());
  //  static_assert(quf == ta<Com1, Com2>());

  //  static_assert(deduce_constructor_arg_types(ta<Foo>(), ta<Com1, Com0>()) ==
  //                ta<Com1, null_type>());

  static_assert(injecxx::detail::is_lazy(meta::t<lazy<int>>));
  static_assert(!injecxx::detail::is_lazy(meta::t<int>));

  auto lazyf = ctx.get<lazy<Foo>>();
  auto& f = ctx.get<Foo>();
  ctx.get<ComG>();
  auto& c2 = ctx.get<Com2>();
  if (&f.c2_ == &c2) {
    std::cout << "Addresses are the same (ok)\n";
  }
  if (&f == &lazyf.get()) {
    std::cout << "Addresses are the same (ok)\n";
  }
  lazyf.get().c2_.act();

  static_assert(ta<int, float>() == t<int> + t<float>);

  static_assert(std::is_same_v<ta_single_t<ta<int, float>>, null_type>);
  static_assert(std::is_same_v<ta_single_t<ta<int>>, int>);

  static_assert(contains(ta<float, int, short>(), t<int>));
  static_assert(!contains(ta<float, int, short>(), t<double>));

  constexpr auto filtered =
      filter(ta<int, short, float>(), meta::predicate<std::is_integral>());
  static_assert(filtered == ta<int, short>());
}
