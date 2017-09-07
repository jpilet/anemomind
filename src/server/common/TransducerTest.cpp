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
