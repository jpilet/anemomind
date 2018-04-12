/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <cmath>
#include <server/common/Functional.h>
#include <server/common/Span.h>
#include <server/common/string.h>

using namespace sail;

TEST(FunctionalTest, TestComplement) {
  auto isOdd = [](int i) {return i % 2 == 1;};

  EXPECT_TRUE(isOdd(119));
  auto isEven = complementFunction(isOdd);

  EXPECT_FALSE(isEven(119));
}
