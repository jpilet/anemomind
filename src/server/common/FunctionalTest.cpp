/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/common/Functional.h>
#include <server/common/Span.h>
#include <server/common/string.h>

using namespace sail;

TEST(FunctionalTest, Map) {
  Arrayd X{1, 2, 3, 4, 5};
  auto negate = [](double x) {return -x;};
  auto odd = [](int i) {return i % 2;};
  auto Xneg = map(negate, X);
  auto Xneg2 = map(negate, Spani(1, 6));
  EXPECT_EQ(toArray(Xneg), (Arrayd{-1, -2, -3, -4, -5}));
  EXPECT_EQ(toArray(Xneg2), (Arrayd{-1, -2, -3, -4, -5}));
  EXPECT_EQ(filter(odd, Spani(1, 6)), (Arrayi{1, 3, 5}));
  auto sum = reduce([](int a, int b) {return a + b;}, (Arrayi{1, 2, 3, 4}));
  EXPECT_EQ(sum, 10);
  auto sum2 = reduce([](int a, int b) {return a + b;}, (Arrayi{1, 2, 3, 4}), 1000);
  EXPECT_EQ(sum2, 1010);

  auto result = map([](double a, double b) {return a + b;},
      map([](double x) {return 1000*x;}, Spani(1, 6)), Spani(1, 6));
  EXPECT_EQ(toArray(result), (Arrayd{1001, 2002, 3003, 4004, 5005}));
}

