/*
 * TransducerTest.cpp
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/Transducer.h>
#include <math.h>

using namespace sail;

// Just a naive example
class VectorStepFunction : public TransducerStep<std::vector<int>, int> {
public:
  std::vector<int> complete(std::vector<int> r) {
    return r;
  }

  std::vector<int> step(std::vector<int> r, int x) {
    r.push_back(x);
    return r;
  }
};

bool isOdd(int i) {
  return i % 2 == 1;
}

bool isEven(int i) {
  return !isOdd(i);
}

TEST(TransducerTest, BasicTest) {
  VectorStepFunction step;

  std::vector<int> result;

  result = step.step(result, 9);

  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 9);

  Map<int, double> m([](double x) {
    return int(round(2.0*x));
  });

  auto doublingFloatStep = m.apply(step);

  result = doublingFloatStep.step(result, 2.5);
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[1], 5);

  Filter<int> f(&isEven);

  // First double incoming elements and round them off,
  // then filter odd elements
  auto mf = composeTransducers(m, f);

  auto mfStep = mf.apply(step);

  std::vector<int> result2;
  result2 = mfStep.step(result2, 1);
  result2 = mfStep.step(result2, 2);
  result2 = mfStep.step(result2, 2.5);
  result2 = mfStep.step(result2, 9.5);
  result2 = mfStep.step(result2, 12);

  EXPECT_EQ(result2.size(), 3);
  EXPECT_EQ(result2[2], 24);
}

class SumStep : public TransducerStep<int, int> {
public:
  int complete(int i) {return i;}
  int step(int a, int b) {return a + b;}
};

TEST(TransducerTest, MoreTesting) {
  std::vector<int> input{9, 34, 5, 6, 7, 8, 1, 3, 3, 4};

  auto T = composeTransducers(
      Filter<int>([](int i) {return i % 2 == 1;}),
      Map<int, int>([](int i) {return i*i;}));

  std::vector<int> result;
  auto init = std::inserter(result, result.end());
  auto basicStep = iteratorStep(init);
  reduce(T.apply(basicStep), init, input);
  EXPECT_EQ(result.size(), 6);
  EXPECT_EQ(result[5], 9);

  int sum = reduce(T.apply(SumStep()), 0, input);

  int sum2 = 0;
  for (auto x: result) {
    sum2 += x;
  }
  EXPECT_EQ(sum, sum2);
}



TEST(TransducerTest, CatTest) {
  std::vector<std::vector<int>> input{
    {}, {1}, {9, 2, 3, 4, 5, 6}, {2, 2},
    {3}, {}, {}, {4}, {}, {9, 9, 9}
  };

  auto T = composeTransducers(
      Filter<std::vector<int>>([](const std::vector<int>& v) {
        return 2 <= v.size();
      }),
      Cat<std::vector<int>>());

  int sum = reduce(T.apply(SumStep()), 0, input);

  EXPECT_EQ(9 + 2 + 3 + 4 + 5 + 6 + 2 +2 + 9 + 9 + 9, sum);
}

std::pair<int, int> pair2(int i) {
  return {i, i};
}

TEST(TransducerTest, SpanTest) {
  std::vector<int> input{0, 0, 0, 1, 0, 0, 3, 3, 3, 4};
  std::vector<SpanWithCount<int>> result;
  auto T = FindSpans<int>([](int a, int b) {return a == b;});
  auto iter = std::inserter(result, result.end());
  reduce(T.apply(iteratorStep(iter)), iter, input);

  EXPECT_EQ(result.size(), 5);
  EXPECT_EQ(result[0].first, 0);
  EXPECT_EQ(result[1].first, 1);
  EXPECT_EQ(result[2].first, 0);
  EXPECT_EQ(result[3].first, 3);
  EXPECT_EQ(result[4].first, 4);
}

bool eq(const Array<int>& a, const Array<int>& b) {
  if (a.size() == b.size()) {
    int n = a.size();
    for (int i = 0; i < n; i++) {
      if (a[i] != b[i]) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

TEST(TransducerTest, Bundle) {
  auto T = Bundle<int>(&isOdd);

  std::vector<int> input{0, 2, 3, 4, 6, 9, 2, 2, 2};
  std::vector<Array<int>> dst;
  auto iter = std::inserter(dst, dst.end());
  reduce(T.apply(iteratorStep(iter)), iter, input);
  EXPECT_EQ(dst.size(), 2);
  EXPECT_TRUE(eq(dst[0], {3, 4, 6}));
  EXPECT_TRUE(eq(dst[1], {9, 2, 2, 2}));
}

