/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/CalibrationModel.h>
#include <gtest/gtest.h>
#include <server/common/Array.h>

using namespace sail;

TEST(CalibModelTest, CountTest) {
  CorrectorSet<double> set = CorrectorSet<double>::makeDefaultCorrectorSet();
  EXPECT_EQ(set.paramCount(), 1 + 1 + 4 + 4 + 2);
}


TEST(CalibModelTest, InitTest) {
  CorrectorSet<double> set = CorrectorSet<double>::makeDefaultCorrectorSet();
  Array<double> params = Array<double>::fill(set.paramCount(), 1.0e6);
  set.initialize(params.ptr());
  for (double p : params) {
    EXPECT_LT(std::abs(p), 1.0e2);
  }
}



