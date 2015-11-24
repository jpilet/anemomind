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
  auto Xneg = map(X, negate);
  auto Xneg2 = map(Spani(1, 6), negate);
  EXPECT_EQ(toArray(Xneg), (Arrayd{-1, -2, -3, -4, -5}));
  EXPECT_EQ(toArray(Xneg2), (Arrayd{-1, -2, -3, -4, -5}));
  EXPECT_EQ(filter(Spani(1, 6), odd), (Arrayi{1, 3, 5}));
  auto sum = reduce((Arrayi{1, 2, 3, 4}), [](int a, int b) {return a + b;});
  EXPECT_EQ(sum, 10);
  auto sum2 = reduce(1000, (Arrayi{1, 2, 3, 4}), [](int a, int b) {return a + b;});
  EXPECT_EQ(sum2, 1010);

  auto result = map(Spani(1, 6),
                    map(Spani(1, 6),
                        [](double x) {return 1000*x;}),
                    [](double a, double b) {return a + b;});
  Arrayd A = toArray(result);
  Arrayd B{1001, 2002, 3003, 4004, 5005};
  EXPECT_EQ(A, B);
}

