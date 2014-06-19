// (c) 2014 Julien Pilet <julien.pilet@gmail.com>

#include <device/Arduino/libraries/TargetSpeed/FixedPoint.h>

#include <gtest/gtest.h>

TEST(FixedPointTest, Conversions) {
  EXPECT_NEAR(103.44f, (float)FP8_8(103.44f), 1/256.0f);
  EXPECT_NEAR(-73.44f, (double)FP8_8(-73.44f), 1/256.0f);
  EXPECT_EQ(-126, (int)FP8_8(-126));
}

TEST(FixedPointTest, ConstOperators) {
  const FP16_16 a(-7.3);
  const FP16_16 b(16.8);
  const double da(a);
  const double db(b);

  EXPECT_NEAR((double)(a + b), da + db, 1/256.0);
  EXPECT_NEAR((double)(a - b), da - db, 1/256.0);
  EXPECT_NEAR((double)(a * b), da * db, 1/256.0);
  EXPECT_NEAR((double)(b / a), db / da, 1/256.0);

  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a >= b);
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a == a);
}

TEST(FixedPointTest, InPlaceOperators) {
  FP16_16 a(87.3);
  FP16_16 b(-3.8);
  double da(a);
  double db(b);

  a += b;
  da += db;
  EXPECT_NEAR((double) a, da, 1/256.0);

  a -= b;
  da -= db;
  EXPECT_NEAR((double) a, da, 1/256.0);

  a *= b;
  da *= db;
  EXPECT_NEAR((double) a, da, 1/256.0);

  a /= b;
  da /= db;
  EXPECT_NEAR((double) a, da, 1/256.0);
}

TEST(FixedPointTest, FixedPointToFixedPointConversions) {
  FP16_16 a(-3.2);
  EXPECT_NEAR((double) a, (double) FP8_8::convert(a), 1/256.0);
  FixedPoint<char, short, 2> b(-3.25);
  EXPECT_NEAR((double) b, (double) FP8_8::convert(b), 1/256.0);
}
  
