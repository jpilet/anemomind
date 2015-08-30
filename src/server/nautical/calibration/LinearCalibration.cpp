/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/calibration/LinearCalibration.h>
#include <armadillo>
#include <server/math/BandMat.h>
#include <server/common/ArrayIO.h>


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

CommonResults calibrateSparse(FlowMatrices mats, Duration<double> totalDuration, CommonCalibrationSettings settings) {
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
    return CommonResults();
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
  return CommonResults{motions, parameters};
}

LinearCorrector::LinearCorrector(const FlowSettings &flowSettings,
    Arrayd windParams, Arrayd currentParams) :
    _flowSettings(flowSettings), _windParams(windParams), _currentParams(currentParams) {}

Array<CalibratedNav<double> > LinearCorrector::operator()(const Array<Nav> &navs) const {
  return navs.map<CalibratedNav<double> >([&](const Nav &x) {
    return (*this)(x);
  });
}

arma::mat asMatrix(const MDArray2d &x) {
  return arma::mat(x.ptr(), x.rows(), x.cols(), false, true);
}

arma::mat asMatrix(const Arrayd &x) {
  return arma::mat(x.ptr(), x.size(), 1, false, true);
}

CalibratedNav<double> LinearCorrector::operator()(const Nav &nav) const {
  static MDArray2d Aw, Bw, Ac, Bc;
  Aw.create(2, _flowSettings.windParamCount()); Bw.create(2, 1);
  Ac.create(2, _flowSettings.currentParamCount()); Bc.create(2, 1);

  makeTrueWindMatrixExpression(nav, _flowSettings, &Aw, &Bw);
  makeTrueCurrentMatrixExpression(nav, _flowSettings, &Ac, &Bc);
  arma::vec2 windMat = asMatrix(Aw)*asMatrix(_windParams) + asMatrix(Bw);
  arma::vec2 currentMat = asMatrix(Ac)*asMatrix(_currentParams) + asMatrix(Bc);

  HorizontalMotion<double> wind{Velocity<double>::knots(windMat[0]),
    Velocity<double>::knots(windMat[1])};
  HorizontalMotion<double> current{Velocity<double>::knots(currentMat[0]),
    Velocity<double>::knots(currentMat[1])};
  CalibratedNav<double> dst;
  dst.rawAwa.set(nav.awa());
  dst.rawAws.set(nav.aws());
  dst.rawMagHdg.set(nav.magHdg());
  dst.rawWatSpeed.set(nav.watSpeed());
  dst.gpsMotion.set(nav.gpsMotion());

  // TODO: Fill in more things here.

  dst.trueWindOverGround.set(wind);
  dst.trueCurrentOverGround.set(current);
  return dst;
}

std::string LinearCorrector::toString() const {
  std::stringstream ss;
  ss << "LinearCorrector(windParams="<< _windParams << ", currentParams=" << _currentParams << ")";
  return ss.str();
}


Results calibrate(CommonCalibrationSettings commonSettings,
    FlowSettings flowSettings, Array<Nav> navs) {
    assert(std::is_sorted(navs.begin(), navs.end()));
    auto totalDuration = navs.last().time() - navs.first().time();
    auto windResults = calibrateSparse(makeTrueWindMatrices(navs, flowSettings),
        totalDuration, commonSettings);
    auto currentResults = calibrateSparse(makeTrueCurrentMatrices(navs, flowSettings),
        totalDuration, commonSettings);
    LinearCorrector corrector(flowSettings, windResults.parameters, currentResults.parameters);
    return Results{corrector, windResults.recoveredFlow, currentResults.recoveredFlow};
}



}
}
