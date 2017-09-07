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

TEST(TransducerTest, MapTest) {
  std::vector<int> src{1, 2, 3, 4, 5, 6};
  std::vector<int> dst;
  transduceIntoColl(map(&mulBy2), &dst, src);
  EXPECT_EQ(dst.size(), 6);
  for (int i = 0; i < 6; i++) {
    EXPECT_EQ(2*src[i], dst[i]);
  }
}

bool isOdd(int x) {return x % 2 == 1;}

TEST(TransducerTest, FilterTest) {
  std::vector<int> src{1, 2, 3, 4, 5, 6};
  std::vector<int> dst;
  transduceIntoColl(filter(&isOdd), &dst, src);
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], 1);
  EXPECT_EQ(dst[1], 3);
  EXPECT_EQ(dst[2], 5);
}

TEST(TransducerTest, ComposeTest) {
  std::vector<int> src{1, 2, 3, 4, 5, 6};
  std::vector<int> dst;
  transduceIntoColl(
      composeTransducers(
          filter(&isOdd),
          map(&mulBy2)), &dst, src);
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], 2);
  EXPECT_EQ(dst[1], 6);
  EXPECT_EQ(dst[2], 10);
}

template <typename T>
class MyBundler {
public:
  MyBundler(std::function<bool(T, T)> f) : _separate(f) {}

  template <typename Acc>
  Step<Acc, T> operator()(const Step<Acc, std::vector<T>>& s) const {
    auto current = std::make_shared<std::vector<T>>();
    auto separate = _separate;
    return {
      [current, s, separate](Acc a, T x) {
        if (current->empty() || !separate(current->back(), x)) {
          current->push_back(x);
          return a;
        } else {
          auto result = s.step(a, *current);
          *current = {x};
          return result;
        }
      },
      [current, s](Acc a) {
        return s.flush(current->empty()? a : s.step(a, *current));
      }
    };
  }
private:
  std::function<bool(T, T)> _separate;
};

bool notEqual(int a, int b) {return a != b;}

bool sufficientlyLong(const std::vector<int>& x) {
  return 2 <= x.size();
}

TEST(TransducerTest, TestFlush) {
  std::vector<int> src{1, 1, 1, 1, 9, 2, 2, 2, 3, 3, 3};
  std::vector<std::vector<int>> dst;
  transduceIntoColl(
      composeTransducers(
          MyBundler<int>(&notEqual),
          Filter<std::function<bool(std::vector<int>)>>(
              &sufficientlyLong)), &dst, src);
  EXPECT_EQ(dst.size(), 3);
  EXPECT_EQ(dst[0], (std::vector<int>{1, 1, 1, 1}));
  EXPECT_EQ(dst[1], (std::vector<int>{2, 2, 2}));
  EXPECT_EQ(dst[2], (std::vector<int>{3, 3, 3}));
}
