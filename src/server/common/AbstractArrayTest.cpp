/*
 * AbstractArray.cpp
 *
 *  Created on: May 13, 2016
 *      Author: jonas
 */

#include <server/common/AbstractArray.h>
#include <gtest/gtest.h>
#include <memory>

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
