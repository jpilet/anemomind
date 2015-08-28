/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/calibration/LinearCalibration.h>
#include <armadillo>
#include <server/math/BandMat.h>




namespace sail {
namespace LinearCalibration {

HorizontalMotion<double> calcRawBoatMotion(
    Angle<double> magHdg, Velocity<double> watSpeed) {
    return HorizontalMotion<double>::polar(watSpeed, magHdg);
}

HorizontalMotion<double> calcRawTrueWind(
    Angle<double> magHdg,
    Angle<double> awa, Velocity<double> aws) {

  /*
   * awa is the angle relative to the boat.
   * awa + magHdg is the angle of the wind w.r.t. the geographic coordinate system.
   * awa + magHdg + 180degs is the angle in which the wind is blowing, not where it comes from
   */
  Angle<double> absoluteDirectionOfWind = magHdg + awa + Angle<double>::degrees(180.0);
  return HorizontalMotion<double>::polar(aws, absoluteDirectionOfWind);
}

void initializeLinearParameters(bool withOffset, double *dst2or4) {
  dst2or4[0] = 1.0;
  int n = (withOffset? 4 : 2);
  for (int i = 1; i < n; i++) {
    dst2or4[i] = 0.0;
  }
}


typedef Eigen::Triplet<double> Triplet;

Results calibrateSparse(FlowMatrices mats, Duration<double> totalDuration, Settings settings) {
  assert(mats.A.rows() == mats.B.rows());
  assert(mats.B.cols() == 1);
  int flowDim = mats.A.rows();
  int flowCount = flowDim/2;
  assert(2*flowCount == flowDim);
  int paramDim = mats.A.cols();
  int regCount = flowCount - settings.regOrder;
  int regDim = 2*regCount;
  int dstCols = flowDim + paramDim;
  int dstRows = flowDim + regDim;
  auto regCoefs = BandMatInternal::makeCoefs(settings.regOrder);
  auto localRegCols = 2*regCoefs.size();

  // The Adst matrix has this structure. It consists of four sub matrices,
  // aligned in two rows and two columns.
  // Adst = [I -mats.A; Reg 0]
  // The Bdst matrix has this structure. It is split into an upper and a lower matrix:
  // Bdst = [mats.B; 0]

  std::vector<Triplet> Adst;
  Eigen::VectorXd Bdst = Eigen::VectorXd::Zero(dstRows);
  Array<Spani> spans(regCount);


  for (int i = 0; i < flowDim; i++) {
    // Build the upper-left part of Adst
    Adst.push_back(Triplet(i, i, 1.0));

    // and fill the upper part of Bdst
    Bdst(i) = mats.B(i, 0);

    // Build the upper right part of Adst
    for (int j = 0; j < paramDim; j++) {
      Adst.push_back(Triplet(i, flowDim + j, -mats.A(i, j)));
    }
  }

  // Fill in the regularization coefficients of the lower left part of Adst
  // Thanks to sparsity, most of their residuals will be more or less exactly 0
  // once the problem is solved.
  for (int i = 0; i < regCount; i++) {
    int offset = 2*i;
    int rowOffset = flowDim + offset;
    spans[i] = Spani(rowOffset, rowOffset + 2);
    for (int j = 0; j < regCoefs.size(); i++) {
      int localCol = offset + 2*j;
      auto c = regCoefs[j];
      Adst.push_back(Triplet(rowOffset + 0, localCol + 0, c));
      Adst.push_back(Triplet(rowOffset + 1, localCol + 1, c));
    }
  }

  Eigen::SparseMatrix<double> AdstMat(dstRows, dstCols);
  AdstMat.setFromTriplets(Adst.begin(), Adst.end());

  // How many non-zero regularization residuals that we allow for.
  // Proportional to the total duration.
  int passiveCount = 1 + int(floor(totalDuration/settings.nonZeroPeriod));
  int activeCount = std::max(regCount - passiveCount, 0);
  auto flowAndParametersVector = SparsityConstrained::solve(AdstMat, Bdst, spans,
      activeCount, settings.spcst);
  if (flowAndParametersVector.size() == 0) {
    return Results();
  }
  assert(flowAndParametersVector.size() == dstCols);
  Arrayd flowAndParameters(dstCols, flowAndParametersVector.data());
  Arrayd parameters = flowAndParameters.sliceFrom(flowDim).dup();
  Arrayd flowData = flowAndParameters.sliceTo(flowDim);
  Array<HorizontalMotion<double> > motions(flowCount);
  for (int i = 0; i < flowCount; i++) {
    int offset = 2*i;
    auto mx = Velocity<double>::knots(flowData[offset + 0]);
    auto my = Velocity<double>::knots(flowData[offset + 1]);
    motions[i] = HorizontalMotion<double>{mx, my};
  }
  return Results{motions, parameters};
}


}
}
