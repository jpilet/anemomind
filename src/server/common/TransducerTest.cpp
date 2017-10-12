/*
 * TransducerTest.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/Transducer.h>
#include <server/common/BundleTransducer.h>

using namespace sail;

int mulBy2(int x) {return 2*x;};

std::vector<int> src{1, 2, 3, 4, 5, 6};

TEST(TransducerTest, Basics) {
  std::vector<int> dst;
  auto iter = std::back_inserter(dst);
  auto step = iteratorStep(iter);
  static_assert(std::is_same<decltype(step)::input_type, int>::value, "");
  static_assert(std::is_same<decltype(step)::result_type, decltype(iter)>::value, "");

  auto m = trMap(&mulBy2);
  transduceIntoColl(m, &dst, src);
  EXPECT_EQ(dst.size(), 6);
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(2*src[i], dst[i]);
  }
}

TEST(TransducerTest, Composition) {
  auto m2 = trMap(&mulBy2) | trMap(&mulBy2) | trMap(&mulBy2);

  std::vector<int> dst;
  transduceIntoColl(m2, &dst, src);

  EXPECT_EQ(dst.size(), 6);
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(8*src[i], dst[i]);
  }
}


bool isOdd(int x) {return x % 2 == 1;}

TEST(TransducerTest, FilterTest) {
  std::vector<int> src{1, 2, 3, 4, 5, 6};
  std::vector<int> dst;
  transduceIntoColl(trFilter(&isOdd), &dst, src);
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], 1);
  EXPECT_EQ(dst[1], 3);
  EXPECT_EQ(dst[2], 5);
}


TEST(TransducerTest, ComposeTest) {
  std::vector<int> src{1, 2, 3, 4, 5, 6};
  std::vector<int> dst;
  transduceIntoColl(
      trFilter(&isOdd) | trMap(&mulBy2),
      &dst, src);
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], 2);
  EXPECT_EQ(dst[1], 6);
  EXPECT_EQ(dst[2], 10);
}

// Example of a custom stateful transducer with a
// non-trivial flush function.
template <typename T, typename Step>
class MyBundler : public Step, public Transducer<MyBundler<T, Step>> {
public:
  MyBundler() {}
  MyBundler(
      std::function<bool(T, T)> f,
      const Step& s = Step()) : _separate(f), Step(s) {}

  template <typename S>
  MyBundler<T, S> apply(const S& step) const {
    return MyBundler<T, S>(_separate, step);
  }

  typedef typename Step::input_type input_type;
  typedef typename Step::result_type result_type;

  result_type step(result_type r, T x) {
    if (_current.empty() || !_separate(_current.back(), x)) {
      _current.push_back(x);
      return r;
    } else {
      auto result = Step::step(r, _current);
      _current = {x};
      return result;
    }
  }

  // If there is something in current, flush it
  result_type flush(result_type r) {
    return Step::flush(_current.empty()?
        r : Step::step(r, _current));
  }
private:
  std::function<bool(T, T)> _separate;
  std::vector<T> _current;
};

bool notEqual(int a, int b) {return a != b;}

bool sufficientlyLong(const std::vector<int>& x) {
  return 2 <= x.size();
}

TEST(TransducerTest, TestFlush) {
  std::vector<int> src{1, 1, 1, 1, 9, 2, 2, 2, 3, 3, 3};
  std::vector<std::vector<int>> dst;
  transduceIntoColl(
      MyBundler<int, UndefinedStep>(&notEqual) |
      trFilter(&sufficientlyLong), &dst, src);
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], (std::vector<int>{1, 1, 1, 1}));
  EXPECT_EQ(dst[1], (std::vector<int>{2, 2, 2}));
  EXPECT_EQ(dst[2], (std::vector<int>{3, 3, 3}));
}

TEST(TransducerTest, CatTest) {
  std::vector<std::vector<int>> src{{3, 4, 5}, {6, 7}};

  int m = 0;

  std::vector<int> dst;
  transduceIntoColl(
      trCat<std::vector<int>>() | trVisit([&](int) {m++;}), &dst, src);

  EXPECT_EQ(dst.size(), 5);
  EXPECT_EQ(dst[3], 6);
  EXPECT_EQ(m, 5);
}

TEST(TransducerTest, BundleTransducer) {
  std::vector<int> src{0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3};
  auto T = trBundle([](int a, int b) {return a != b;})
           |
           trMap([](const std::vector<int>& x) {
              return x.size();
           });

  auto step = T.apply(CountStep<int>());

  int result = reduce(step, 0, src);
  std::vector<int> dst;
  transduceIntoColl(T, &dst, src);

  EXPECT_EQ(dst.size(), 4);
  EXPECT_EQ(dst[0], 1);
  EXPECT_EQ(dst[1], 3);
  EXPECT_EQ(dst[2], 6);
  EXPECT_EQ(dst[3], 3);
}
