/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/tgtspeed/TargetSpeedParam.h>

#include <gtest/gtest.h>
#include <server/common/ArrayIO.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/string.h>
#include <armadillo>

using namespace sail;

TEST(TargetSpeedParamTest, Mapping) {
  TargetSpeedParam param(true, true,
      3, 2, Velocity<double>::metersPerSecond(20));

  EXPECT_NEAR(0.5, param.windAngleToAngleIndex(Angle<double>::degrees(60)), 1.0e-4);
  EXPECT_NEAR(1.0, param.windAngleToAngleIndex(Angle<double>::degrees(120)), 1.0e-4);
  EXPECT_NEAR(120, param.angleIndexToWindAngle(1.0).degrees(), 1.0e-4);
  EXPECT_NEAR(0.0, param.radiusIndexToWindSpeed(0.0).metersPerSecond(), 1.e-4);
  EXPECT_NEAR(20.0, param.radiusIndexToWindSpeed(1.0).metersPerSecond(), 1.e-4);
  EXPECT_EQ(3, param.totalCellCount());
  EXPECT_EQ(1, param.radiusCellCount());
  EXPECT_EQ(3, param.angleCellCount());
  EXPECT_EQ(1, param.angleParamCount());
  EXPECT_EQ(1, param.radiusParamCount());
  EXPECT_EQ(-1, param.calcAngleParamIndex(0));
  EXPECT_EQ(-1, param.calcAngleParamIndex(3));
  EXPECT_EQ(-1, param.calcAngleParamIndex(3));
  EXPECT_EQ(0, param.calcAngleParamIndex(1));
  EXPECT_EQ(0, param.calcAngleParamIndex(2));
  EXPECT_EQ(0, param.calcRadiusParamIndex(1));
  EXPECT_EQ(-1, param.calcRadiusParamIndex(0));
  EXPECT_EQ(6, param.vertexCount());
}

int countOnesOnRow(MDArray2d row) {
  int count = 0;
  for (int j = 0; j < row.cols(); j++) {
    double value = row(0, j);
    EXPECT_TRUE(value == 0.0 || value == 1.0);
    count += (value == 1.0? 1 : 0);
  }
  return count;
}

TEST(TargetSpeedParamTest, Mapping2) {
  TargetSpeedParam param(true, false,
      6, 3, Velocity<double>::metersPerSecond(20));
  EXPECT_EQ(3, param.angleParamCount());
  EXPECT_EQ(2, param.radiusParamCount());
  EXPECT_EQ(-1, param.calcAngleParamIndex(0));
  EXPECT_EQ(0, param.calcAngleParamIndex(1));
  EXPECT_EQ(1, param.calcAngleParamIndex(2));
  EXPECT_EQ(2, param.calcAngleParamIndex(3));
  EXPECT_EQ(1, param.calcAngleParamIndex(4));
  EXPECT_EQ(0, param.calcAngleParamIndex(5));
  EXPECT_EQ(-1, param.calcAngleParamIndex(6));
  EXPECT_EQ(0, param.calcAngleParamIndex(7));
  EXPECT_EQ(-1, param.calcRadiusParamIndex(0));
  EXPECT_EQ(0, param.calcRadiusParamIndex(1));
  EXPECT_EQ(1, param.calcRadiusParamIndex(2));


  auto P = param.makeNonNegativeVertexParamMatrix();
  int counts[3] = {0, 0, 0};
  for (int i = 0; i < P.rows(); i++) {
    int n = countOnesOnRow(P.sliceRow(i));
    EXPECT_LE(n, 2);
    counts[n]++;
  }
  EXPECT_EQ(counts[1], 5);
  EXPECT_EQ(counts[2], 5);
}

TEST(TargetSpeedParamTest, Mapping3) {
  TargetSpeedParam param(true, false,
      7, 4, Velocity<double>::metersPerSecond(20));
  auto P = param.makeNonNegativeVertexParamMatrix();
  int counts[4] = {0, 0, 0, 0};
  for (int i = 0; i < P.rows(); i++) {
    int n = countOnesOnRow(P.sliceRow(i));
    EXPECT_LE(n, 3);
    counts[n]++;
  }
  EXPECT_EQ(counts[1], 6);
  EXPECT_EQ(counts[2], 6);
  EXPECT_EQ(counts[3], 6);
}

TEST(TargetSpeedParamTest, Symmetry) {
  TargetSpeedParam param(true, false,
      24, 27, Velocity<double>::metersPerSecond(26));

  arma::mat params = arma::zeros(param.paramCount(), 1);
  for (int i = 0; i < params.size(); i++) {
    params(i, 0) = sin(324234*i);
  }
  auto P = param.makeNonNegativeVertexParamMatrix();
  auto Parma = arma::mat(P.ptr(), P.rows(), P.cols(), false, true);
  arma::mat aVertices = Parma*params;
  auto vertices = Arrayd(aVertices.n_elem, aVertices.memptr());
  for (int i = 0; i < 3000; i++) {
    auto windAngle = Angle<double>::degrees(300*sin(399*i));
    auto windSpeed = Velocity<double>::knots(12*(1 + sin(34323*i)));
    auto a = param.interpolate(windAngle, windSpeed, vertices);
    auto b = param.interpolate(-windAngle, windSpeed, vertices);
    EXPECT_NEAR(a, b, 1.0e-6);
  }
}


bool isDifReg(arma::mat X) {
  if (X.n_rows + 1 != X.n_cols) {
    return false;
  }
  double tol = 1.0e-6;
  for (int i = 0; i < X.n_rows; i++) {
    for (int j = 0; j < X.n_cols; j++) {
      if (i == j || i+1 == j) {
        if (std::abs(X(i, j)) < tol) {
          return false;
        }
        if (i == j) {
          if (std::abs(X(i, j) + X(i, j+1)) > tol) {
            return false;
          }
        }
      } else {
        if (std::abs(X(i, j)) > tol) {
          return false;
        }
      }
    }
  }
  return true;
}

bool validVertexIndices(int vertexCount, Arrayi inds) {
  for (auto index: inds) {
    if (index < 0 || vertexCount <= index) {
      return false;
    }
  }
  return true;
}

void checkRegs(TargetSpeedParam p, Array<TargetSpeedParam::SubReg> regs) {
  for (auto i = 0; i < regs.size(); i++) {
    auto r = regs[i];
    EXPECT_TRUE(isDifReg(r.A));
    EXPECT_EQ(r.A.n_cols, r.inds.size());
    EXPECT_TRUE(validVertexIndices(p.vertexCount(), r.inds));
  }
}


TEST(TargetSpeedParamTest, Reg) {
  TargetSpeedParam param(true, false,
      9, 8, Velocity<double>::metersPerSecond(20));

  checkRegs(param, param.makeAngularSubRegs());
  checkRegs(param, param.makeRadialSubRegs());
  auto regs = param.makeRadialSubRegs();
  auto first = regs[0];
  auto next = first.nextOrder(1);
  EXPECT_EQ(next.inds, first.inds);
  EXPECT_EQ(next.A.n_rows + 2, next.A.n_cols);

  auto AtA = param.assembleReg(regs, 5);
  EXPECT_EQ(AtA.n_rows, param.vertexCount());
  EXPECT_EQ(AtA.n_cols, param.vertexCount());
  arma::mat symmetryError = arma::abs(AtA - AtA.t());
  int n = param.vertexCount();
  arma::mat errorSum = arma::ones(1, n)*symmetryError*arma::ones(n, 1);
  double meanError = errorSum(0, 0)/(n*n);
  EXPECT_LT(meanError, 1.0e-6);
}
