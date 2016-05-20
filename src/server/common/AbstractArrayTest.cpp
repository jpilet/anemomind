/*
 * AbstractArray.cpp
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#include <server/common/AbstractArray.h>
#include <gtest/gtest.h>
#include <memory>
#include <algorithm>

using namespace sail;

namespace {
  class MyArray : public AbstractArray<int> {
  public:
    int operator[] (int i) const override {
      return i*i;
    }

    int size() const override {
      return 7;
    }
  };

  std::shared_ptr<AbstractArray<int> > getArray() {
    return std::shared_ptr<AbstractArray<int> >(new MyArray());
  }
}

TEST(AbstractArrayTest, TestMyArray) {
  auto array = getArray();
  EXPECT_EQ((*array)[4], 16);
  EXPECT_EQ(array->size(), 7);
}

TEST(AbstractArrayTest, RangeLoop) {
  int data[7];
  int counter = 0;
  for (auto x: MyArray()) {
    EXPECT_EQ(counter*counter, x);
    counter++;
  }
  EXPECT_EQ(counter, 7);
}

namespace {
  int mySqrt(int i) {
    MyArray arr;
    int diff = std::lower_bound(arr.begin(), arr.end(), i) - arr.begin();
    return diff;
  }

}

TEST(AbstractArrayTest, Sqrt) {
  EXPECT_EQ(mySqrt(9), 3);
}

TEST(AbstractArrayTest, WrapTest) {
  std::vector<int> X{3, 4, 5};
  {
    IndexableWrap<std::vector<int>, TypeMode::Value> w0(X);
    AbstractArray<int> &w = w0;

    EXPECT_EQ(w.size(), 3);
    EXPECT_EQ(w[1], 4);
  }{
    IndexableWrap<std::vector<int>, TypeMode::ConstRef> w0(X);
    AbstractArray<int> &w = w0;

    EXPECT_EQ(w.size(), 3);
    EXPECT_EQ(w[1], 4);
  }

}
