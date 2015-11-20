/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Span.h>
#include <server/nautical/tgtspeed/TargetSpeedParam.h>
#include <server/common/ArrayIO.h>
#include <server/common/math.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>
#include <server/nautical/tgtspeed/table.h>
#include <server/plot/extra.h>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/Functional.h>

namespace sail {

namespace {
  Velocity<double> velocityUnit = Velocity<double>::knots(1.0);
}

TargetSpeedParam::TargetSpeedParam(bool symmetric, bool squaredRadius,
    int totalAngleCount, int totalRadiusCount,
    Velocity<double> maxWindVelocity) : _symmetric(symmetric),
    _squaredRadius(squaredRadius), _totalAngleCount(totalAngleCount),
    _totalRadiusCount(totalRadiusCount), _maxWindSpeed(maxWindVelocity) {
  _windFactor = (1.0/distortRadius(_totalRadiusCount - 1))*maxWindVelocity;
  _angleFactor = (1.0/totalAngleCount)*Angle<double>::degrees(360.0);
}

int TargetSpeedParam::angleParamCount() const {
  int temp = _totalAngleCount - 1;
  return (_symmetric? (temp + 1)/2 : temp);
}

int TargetSpeedParam::radiusParamCount() const {
  return _totalRadiusCount - 1;
}

int TargetSpeedParam::calcRadiusParamIndex(int radiusIndex) const {
  return radiusIndex - 1;
}

TargetSpeedParam::BilinearWeights::BilinearWeights() {
  cellIndex = -1;
  for (int i = 0; i < 4; i++) {
    inds[i] = -1;
    weights[i] = NAN;
  }
}

int TargetSpeedParam::calcAngleParamIndex(int angleIndex) const {
  int temp = positiveMod<int>(angleIndex, _totalAngleCount) - 1;
  if (_symmetric) {
    int ap = angleParamCount();
    if (temp < ap) {
      return temp;
    } else {
      int lastIndex = _totalAngleCount - 2;
      return lastIndex - temp;
    }
  } else {
    return temp;
  }
}

int TargetSpeedParam::calcVertexIndex(int angleIndex, int radiusIndex) const {
  int index = _totalRadiusCount*positiveMod<int>(angleIndex, _totalAngleCount) + radiusIndex;
  assert(0 <= index && index < vertexCount());
  return index;
}

int TargetSpeedParam::calcParamIndex(int angleParamIndex, int radiusParamIndex) const {
  if (angleParamIndex == -1 || radiusParamIndex == -1) {
    return -1;
  }
  return radiusParamCount()*angleParamIndex + radiusParamIndex;
}



Array<HorizontalMotion<double> > TargetSpeedParam::vertexMotions() const {
  Array<HorizontalMotion<double> > motions(vertexCount());
  for (int i = 0; i < _totalAngleCount; i++) {
    for (int j = 0; j < _totalRadiusCount; j++) {
      motions[calcVertexIndex(i, j)] =
          HorizontalMotion<double>::polar(radiusIndexToWindSpeed(double(j)), angleIndexToWindAngle(double(i)));
    }
  }
  return motions;
}



TargetSpeedParam::SubReg TargetSpeedParam::SubReg::nextOrder() const {
  int last = A.n_rows-1;
  return SubReg{inds, A.rows(0, last-1) - A.rows(1, last)};
}

TargetSpeedParam::SubReg TargetSpeedParam::SubReg::nextOrder(int steps) const {
  if (steps <= 0) {
    return *this;
  } else {
    return nextOrder(steps-1).nextOrder();
  }
}


Array<TargetSpeedParam::SubReg> TargetSpeedParam::makeRadialSubRegs() const {
  return toArray(map(
      [&](int angleIndex) {
    int difCount = _totalRadiusCount - 1;
    arma::mat A = arma::zeros(difCount, _totalRadiusCount);
    for (int i = 0; i < difCount; i++) {
      double w = normalizeRadial(i+1);
      A(i, i) = w;
      A(i, i+1) = -w;
    }

    return SubReg{toArray(map([&](int radiusIndex) {
      return calcVertexIndex(angleIndex, radiusIndex);
    }, Spani(0, _totalRadiusCount))), A};
  }, Spani(1, _totalAngleCount)));
}



Array<TargetSpeedParam::SubReg> TargetSpeedParam::makeAngularSubRegs() const {
  return toArray(map([&](int radiusIndex) {
      double w = 1.0/radiusIndexToWindSpeed(double(radiusIndex)).knots();
      arma::mat A = arma::zeros(_totalAngleCount, _totalAngleCount+1);
      for (int i = 0; i < _totalAngleCount; i++) {
        A(i, i) = w;
        A(i, i+1) = -w;
      }
      return SubReg{toArray(map([&](int angleIndex) {
        return calcVertexIndex(angleIndex, radiusIndex);
      }, Spani(0, _totalAngleCount+1))), A};
  }, Spani(1, _totalRadiusCount)));
}

void accumulateReg(arma::mat *dstPtr, TargetSpeedParam::SubReg reg) {
  arma::mat &dst = *dstPtr;
  arma::mat AtA = reg.A.t()*reg.A;
  int n = reg.inds.size();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      dst(reg.inds[i], reg.inds[j]) += AtA(i, j);
    }
  }
}

arma::mat TargetSpeedParam::assembleReg(Array<SubReg> regs, int order) const {
  int n = vertexCount();
  arma::mat dst = arma::zeros(n, n);
  for (int i = 0; i < regs.size(); i++) {
    accumulateReg(&dst, regs[i].nextOrder(order - 1));
  }
  return dst;
}


MDArray2d TargetSpeedFunction::samplePolarCurve2d(
    Velocity<double> windSpeed, int sampleCount) const {
  LineKM angle(0, sampleCount-1, 0.0, 360.0);
  MDArray2d xy(sampleCount, 2);
  for (int i = 0; i < sampleCount; i++) {
    auto alpha = Angle<double>::degrees(angle(i));

    auto boatSpeed = _param.calcBilinearWeights(alpha, windSpeed).eval(_vertices);
    auto motion = HorizontalMotion<double>::polar(windSpeed, alpha);
    auto pt = HorizontalMotion<double>::polar(
        boatSpeed,
        alpha);
    xy(i, 0) = pt[0].knots();
    xy(i, 1) = pt[1].knots();
  }
  return xy;
}

MDArray2d TargetSpeedFunction::samplePolarCurve3d(
    Velocity<double> windSpeed, int sampleCount) const {
  LineKM angle(0, sampleCount-1, 0.0, 360.0);
  MDArray2d xyz(sampleCount, 3);
  for (int i = 0; i < sampleCount; i++) {
    auto alpha = Angle<double>::degrees(angle(i));
    auto motion = HorizontalMotion<double>::polar(windSpeed, alpha);
    xyz(i, 0) = motion[0].knots();
    xyz(i, 1) = motion[1].knots();

    auto z = _param.calcBilinearWeights(alpha, windSpeed).eval(_vertices);
    xyz(i, 2) = z.knots();
  }
  return xyz;
}




TargetSpeedParam::BilinearWeights TargetSpeedParam::calcBilinearWeights(Angle<double> windAngle,
  Velocity<double> windSpeed) const {
  BilinearWeights bw;
  bw.cellIndex = calcVertexLinearCombination(windAngle, windSpeed, bw.inds, bw.weights);
  return bw;
}

int TargetSpeedParam::calcVertexLinearCombination(
    Angle<double> angle, Velocity<double> radius,
    int *outInds, double *outWeights) const {
  return calcVertexLinearCombination(windAngleToAngleIndex(angle),
      windSpeedToRadiusIndex(radius), outInds, outWeights);
}


int TargetSpeedParam::calcVertexLinearCombination(
    double angleIndex0, double radiusIndex0,
    int *outInds, double *outWeights) const {
    int angleIndex = int(floor(angleIndex0));
    double angleFrac = angleIndex0 - angleIndex;
    int radiusIndex = int(floor(radiusIndex0));
    double radiusFrac = radiusIndex0 - radiusIndex;
    int cellIndex = calcCellIndex(angleIndex, radiusIndex);
    if (cellIndex == -1) {
      return cellIndex;
    }
    outInds[0] = calcVertexIndex(angleIndex + 0, radiusIndex + 0);
    outInds[1] = calcVertexIndex(angleIndex + 0, radiusIndex + 1);
    outInds[2] = calcVertexIndex(angleIndex + 1, radiusIndex + 0);
    outInds[3] = calcVertexIndex(angleIndex + 1, radiusIndex + 1);
    outWeights[0] = (1.0 - angleFrac)*(1.0 - radiusFrac);
    outWeights[1] = (1.0 - angleFrac)*radiusFrac;
    outWeights[2] = angleFrac*(1.0 - radiusFrac);
    outWeights[3] = angleFrac*radiusFrac;
    return cellIndex;
}

Array<MDArray2d> TargetSpeedFunction::samplePolarCurves(bool dims3) const {
  int count = _param.totalRadiusCount() - 2;
  Array<MDArray2d> curves(count);
  for (int i = 0; i < count; i++) {
    auto windSpeed = _param.radiusIndexToWindSpeed(double(i + 1));
    curves[i] = (dims3? samplePolarCurve3d(windSpeed) : samplePolarCurve2d(windSpeed));
  }
  return curves;
}

Array<MatrixElementd> TargetSpeedParam::makeNonNegativeVertexParam() const {
  ArrayBuilder<MatrixElementd> builder;
  int vc = vertexCount();
  for (int angleIndex = 0; angleIndex < _totalAngleCount; angleIndex++) {
    int angleParamIndex = calcAngleParamIndex(angleIndex);
    if (angleParamIndex != -1) {
      for (int radiusIndex = 0; radiusIndex < _totalRadiusCount; radiusIndex++) {
        int radiusParamIndex = calcRadiusParamIndex(radiusIndex);
        if (radiusParamIndex != -1) {
          int row = calcVertexIndex(angleIndex, radiusIndex);
          for (int i = 0; i <= radiusParamIndex; i++) {
            int col = calcParamIndex(angleParamIndex, i);
            builder.add(MatrixElementd{row, col, 1.0});
          }
        }
      }
    }
  }
  return builder.get();
}

MDArray2d TargetSpeedParam::makeNonNegativeVertexParamMatrix() const {
  MDArray2d m(vertexCount(), paramCount());
  m.setAll(0.0);
  auto elements = makeNonNegativeVertexParam();
  for (auto e: elements) {
    m(e.i, e.j) = e.value;
  }
  return m;
}

Arrayd TargetSpeedParam::initializeNonNegativeParams() const {
  return Arrayd::fill(paramCount(), 0.3*_maxWindSpeed.knots()/radiusParamCount());
}

TargetSpeedParam::TargetSpeedParam() :
    _totalAngleCount(-1), _totalRadiusCount(-1), _symmetric(false),
    _squaredRadius(false) {}

TargetSpeedFunction::TargetSpeedFunction(TargetSpeedParam param,
    Array<Velocity<double> > vertices) :
  _param(param), _vertices(vertices) {
  assert(vertices.size() == param.vertexCount() || vertices.empty());
}
bool TargetSpeedFunction::defined() const {
  return !_vertices.empty();
}

void TargetSpeedFunction::outputTextTable(Array<Angle<double> > windAngles,
    Array<Velocity<double> > windSpeeds, std::ostream *dst) const {
  outputTWSHeader(windSpeeds, dst);
  for (auto angle: windAngles) {
    outputTWALabel(angle, dst);
    for (auto speed: windSpeeds) {
      auto bs = _param.calcBilinearWeights(angle, speed).eval(_vertices);
      outputValue(bs.knots(), dst);
    }
    *dst << "\n";
  }
}

void TargetSpeedFunction::outputNorthSailsTable(std::ostream *out) const {
  auto windSpeeds = getNorthSailsSpeeds();
  auto windAngles = getNorthSailsAngles();
  outputTextTable(windAngles, windSpeeds, out);
}


void TargetSpeedFunction::plotPolarCurves(
    bool dims3, Velocity<double> unit) const {
  auto curves = samplePolarCurves(dims3);
  GnuplotExtra plot;
  plot.set_style("lines");
  for (int i = 0; i < curves.size(); i++) {
    plot.plot(curves[i]);
  }
  plot.show();
}

Velocity<double> TargetSpeedFunction::calcBoatSpeed(Angle<double> windAngle,
    Velocity<double> windSpeed) const {
    return _param.calcBilinearWeights(windAngle, windSpeed).eval(_vertices);
}









}
