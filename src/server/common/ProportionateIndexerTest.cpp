/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/ProportionateIndexer.h>

using namespace sail;

TEST(PropTest, Test) {
  Arrayd arr(3);
  arr[0] = 1.0;
  arr[1] = 2.0;
  arr[2] = 1.0;
  ProportionateIndexer s(arr);
  EXPECT_NEAR(3.3, s.get(3.3).x, 1.0e-6);
  {
    Arrayb remaining = s.remaining();
    Arrayb selected = s.selected();
    for (int i = 0; i < 3; i++) {
      EXPECT_FALSE(selected[i]);
      EXPECT_TRUE(remaining[i]);
    }
  }
  EXPECT_EQ(0, s.get(0.5).index);
  EXPECT_EQ(1, s.get(1.5).index);
  EXPECT_EQ(1, s.get(2.5).index);
  EXPECT_EQ(2, s.get(3.5).index);
  s.remove(1);
  EXPECT_EQ(0, s.get(0.25*s.sum()).index);
  EXPECT_EQ(2, s.get(0.75*s.sum()).index);
  s.remove(0);
  for (double x = 0.01; x < 1; x += 0.032344) {
    EXPECT_EQ(2, s.get(x).index);
  }
  s.remove(2);

  {
    Arrayb remaining = s.remaining();
    Arrayb selected = s.selected();
    Arrayd props = s.proportions();
    for (int i = 0; i < 3; i++) {
      EXPECT_TRUE(selected[i]);
      EXPECT_FALSE(remaining[i]);
      EXPECT_NEAR(props[i], 0.0, 1.0e-6);
    }
  }
}

TEST(PropTest, RoundOffError) {
  const double huge = 1.34332e30;
  const double tiny = 9.239e-12;

  Arrayd arr = Arrayd{huge, tiny};

  ProportionateIndexer indexer(arr);

  // Due to cancellation, the tiny number will be treated as 0.
  EXPECT_EQ(indexer.sum(), huge);

  // The huge number is removed...
  indexer.remove(0);

  // Check that the cancellation effect is gone after the huge number is removed,
  // and the sum is that of the tiny number.
  EXPECT_EQ(indexer.sum(), tiny);
}
