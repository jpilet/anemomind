/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/ProportionateSampler.h>

using namespace sail;

TEST(PropTest, Test) {
  Arrayd arr(3);
  arr[0] = 1.0;
  arr[1] = 2.0;
  arr[2] = 1.0;
  ProportionateSampler s(arr);
  {
    Arrayb remaining = s.remaining();
    Arrayb selected = s.selected();
    for (int i = 0; i < 3; i++) {
      EXPECT_FALSE(selected[i]);
      EXPECT_TRUE(remaining[i]);
    }
  }
  EXPECT_EQ(0, s.get(0.5/4));
  EXPECT_EQ(1, s.get(1.5/4));
  EXPECT_EQ(1, s.get(2.5/4));
  EXPECT_EQ(2, s.get(3.5/4));
  s.remove(1);
  EXPECT_EQ(0, s.get(0.25));
  EXPECT_EQ(2, s.get(0.75));
  s.remove(0);
  for (double x = 0.01; x < 1; x += 0.032344) {
    EXPECT_EQ(2, s.get(x));
  }

  {
    Arrayb remaining = s.remaining();
    Arrayb selected = s.selected();
    for (int i = 0; i < 3; i++) {
      EXPECT_TRUE(selected[i]);
      EXPECT_FALSE(remaining[i]);
    }
  }
}
