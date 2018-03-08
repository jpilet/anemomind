/*
 * TransducerTest.cpp
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/Transducer.h>

using namespace sail;

int mulBy2(int x) {return 2*x;};

std::vector<int> src{1, 2, 3, 4, 5, 6};

TEST(TransducerTest, Basics) {
  auto dst = transduce(src, trMap(&mulBy2), IntoArray<int>());

  EXPECT_EQ(dst.size(), 6);
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(2*src[i], dst[i]);
  }
}

TEST(TransducerTest, SumTest) {
  auto result = transduce(
      src,
      trIdentity(),
      intoReduction<double>([](double sum, int x) {
        return sum + x;
      }, 1000));
  EXPECT_NEAR(result, 1000 + 21, 1.0e-6);
}

TEST(TransducerTest, Composition) {
  auto dst = transduce(
      src,
      trMap(&mulBy2) | trMap(&mulBy2) | trMap(&mulBy2),
      IntoArray<int>());

  EXPECT_EQ(dst.size(), 6);
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(8*src[i], dst[i]);
  }
}

bool isOdd(int x) {return x % 2 == 1;}

TEST(TransducerTest, FilterTest) {
  std::vector<int> src{1, 2, 3, 4, 5, 6};
  auto dst = transduce(src, trFilter(&isOdd), IntoArray<int>());


  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], 1);
  EXPECT_EQ(dst[1], 3);
  EXPECT_EQ(dst[2], 5);
}


TEST(TransducerTest, ComposeTest) {
  auto dst = transduce(
      src,
      trFilter(&isOdd) | trMap(&mulBy2),
      IntoArray<int>());
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], 2);
  EXPECT_EQ(dst[1], 6);
  EXPECT_EQ(dst[2], 10);
}

// Example of a custom stateful transducer with a
// non-trivial flush function.

template <typename T>
class MyBundleStepper {
public:
  MyBundleStepper(
      std::function<bool(T, T)> f) : _separate(f){}


  template <typename Result, typename X>
  void apply(Result* dst, X x) {
    if (_current.empty() || !_separate(_current.back(), x)) {
      _current.push_back(x);
    } else {
      dst->add(_current);
      _current = {x};
    }
  }

  // If there is something in current, flush it
  template <typename Result>
  void flush(Result* r) {
    if (!_current.empty()) {
      r->add(_current);
    }
    r->flush();
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

  auto dst = transduce(
      src,
      genericTransducer(MyBundleStepper<int>(&notEqual))
          |
      trFilter(&sufficientlyLong),
      IntoArray<std::vector<int>>());

  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], (std::vector<int>{1, 1, 1, 1}));
  EXPECT_EQ(dst[1], (std::vector<int>{2, 2, 2}));
  EXPECT_EQ(dst[2], (std::vector<int>{3, 3, 3}));
}

TEST(TransducerTest, MergeTest) {
  std::vector<int> A{0, 0, 1, 6, 6, 7, 9, 9, 20, 30};
  std::vector<int> B{0, 0, 0, 2, 8, 77};
  std::vector<int> C{5, 6, 7, 8, 9, 10, 11};

  auto result = transduce(
      A, // <-- Use A to drive the process
      trMerge(B.begin(), B.end())
      | // merge(A, B)
      trMerge(C.begin(), C.end()),
        // merge(A, B, C)
      IntoArray<int>());

  auto result2 = transduce(
      A,
      trMergeColl(B)
      |
      trMergeColl(C),
      IntoArray<int>());

  std::vector<int> expected;
  for (auto a: A) {
    expected.push_back(a);
  }
  for (auto b: B) {
    expected.push_back(b);
  }
  for (auto c: C) {
    expected.push_back(c);
  }
  std::sort(expected.begin(), expected.end());

  EXPECT_EQ(result.size(), A.size() + B.size() + C.size());

  for (int i = 0; i < expected.size(); i++) {
    EXPECT_EQ(expected[i], result[i]);
    EXPECT_EQ(expected[i], result2[i]);
  }
}
