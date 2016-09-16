/*
 * MDArrayTest.cpp
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 */

#include <server/common/MDArray.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(MDArrayTest, TestMap) {
  MDArray2d A(3, 4);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      A(i, j) = i*1000 + j;
    }
  }
  auto B = A.map([](double x) {return x + 5.0;});


  auto C = A.cast<int>();

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      EXPECT_EQ(B(i, j), i*1000 + j + 5.0);
      EXPECT_EQ(C(i, j), i*1000 + j);
    }
  }

}



