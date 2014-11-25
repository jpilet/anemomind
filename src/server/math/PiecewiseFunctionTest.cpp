/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/math/PiecewiseFunction.h>
#include <server/common/LineKM.h>
#include <cmath>

using namespace sail;

namespace {
  std::function<int(double)> makeMyRoundToInt() {
    Array<PiecewiseFunction::Piece> pieces(30);
    for (int i = 0; i < 30; i++) {
      pieces[i] = PiecewiseFunction::Piece(1.0, LineKM::constant(i));
    }
    PiecewiseFunction fun(-0.5, pieces);
    return [=](double x) {
      return int(fun(x));
    };
  }
}

TEST(PiecewiseFunctionTest, RoundToInt) {
  std::function<int(double)> myRoundToInt = makeMyRoundToInt();

  int count = 300;
  LineKM map(-1, 1, -0.5, 19);
  for (int i = 0; i < count; i++) {
    double x = map(sin(97.632324*sin(34.034*i)));
    EXPECT_EQ(myRoundToInt(x), int(round(x)));
  }
}
