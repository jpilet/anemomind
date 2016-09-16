/*
 * MDArrayTest.cpp
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 */

#include <server/common/MDArray.h>
#include <gtest/gtest.h>

using namespace sail;

void testWithMatrix(MDArray2d A) {
  auto B = A.map([](double x) {return x + 5.0;});
  auto C = A.cast<int>();
  for (int i = 0; i < A.rows(); i++) {
    for (int j = 0; j < A.cols(); j++) {
      EXPECT_EQ(B(i, j), A(i, j) + 5.0);
      EXPECT_EQ(C(i, j), static_cast<int>(A(i, j)));
    }
  }
}

TEST(MDArrayTest, TestMap) {
  MDArray2d A(3, 4);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      A(i, j) = i*1000 + j;
    }
  }
  testWithMatrix(A);
  testWithMatrix(A.sliceRowsFrom(1));
}



