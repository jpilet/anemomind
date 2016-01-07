/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/Stability.h>
#include <server/common/LineKM.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>
#include <server/common/Functional.h>

using namespace sail;

double f1(double x) {
  if (x < 100) {
    return -3;
  } else if (x < 200) {
    return LineKM(100, 200, -3, 7)(x);
  }
  return 7;
}

double f2(double x) {
  if (x < 300) {
    return 8;
  } else if (x < 400) {
    return LineKM(300, 400, 8, 3)(x);
  }
  return 3;
}

TEST(StabiliyTest, Test2) {
  Arrayd X = toArray(map(Spani(0, 500), [&](int i) {return double(i);}));
  Arrayd Y1 = toArray(map(X, f1));
  Arrayd Y2 = toArray(map(X, f2));

  auto pair1 = std::pair<Arrayd, Arrayd>(X, Y1);
  auto pair2 = std::pair<Arrayd, Arrayd>(X, Y2);

  auto segments = Stability::optimize(Array<std::pair<Arrayd, Arrayd> >{pair1, pair2}, 500, LineKM::identity(), 30);
  class Cmp {
   public:
     bool operator() (const Stability::Segment &a, const Stability::Segment &b) const {
       return a.span.width() > b.span.width();
     }
  };
  std::sort(segments.begin(), segments.end(), Cmp());
  EXPECT_NEAR(segments[0].span.width(), 100, 4);
  EXPECT_NEAR(segments[1].span.width(), 100, 4);
  EXPECT_NEAR(segments[2].span.width(), 100, 4);
  EXPECT_LE(segments[3].span.width(), 40);
  EXPECT_LT(segments[3].totalStability(), segments[0].totalStability());
  auto s = segments[0];
  EXPECT_NEAR(s.span.minv(), 200, 4);
  EXPECT_NEAR(s.span.maxv(), 300, 4);
  EXPECT_NEAR(s.stability, 300, 4);
  EXPECT_NEAR(s.values[0], 7, 0.1);
  EXPECT_NEAR(s.values[1], 8, 0.1);
  EXPECT_NEAR(s.totalStability(), 300*100, 1000);
}
