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
  return Spani(1, _totalAngleCount).map<TargetSpeedParam::SubReg>(
      [&](int angleIndex) {
    int difCount = _totalRadiusCount - 1;
    arma::mat A = arma::zeros(difCount, _totalRadiusCount);
    for (int i = 0; i < difCount; i++) {
      double w = normalizeRadial(i+1);
      A(i, i) = w;
      A(i, i+1) = -w;
    }
    return SubReg{Spani(0, _totalRadiusCount).map<int>([&](int radiusIndex) {
      return calcVertexIndex(angleIndex, radiusIndex);
    }), A};
  });
}



Array<TargetSpeedParam::SubReg> TargetSpeedParam::makeAngularSubRegs() const {
  return Spani(1, _totalRadiusCount).map<TargetSpeedParam::SubReg>(
      [&](int radiusIndex) {
      double w = 1.0/radiusIndexToWindSpeed(double(radiusIndex)).knots();
      arma::mat A = arma::zeros(_totalAngleCount, _totalAngleCount+1);
      for (int i = 0; i < _totalAngleCount; i++) {
        A(i, i) = w;
        A(i, i+1) = -w;
      }
      return SubReg{Spani(0, _totalAngleCount+1).map<int>([&](int angleIndex) {
        return calcVertexIndex(angleIndex, radiusIndex);
      }), A};
  });
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

void dispLoc(TargetSpeedParam p, Angle<double> a, Velocity<double> s) {
  auto l = p.calcLoc(a, s);
  Arrayi inds(4);
  Arrayd weights(4);
  p.calcVertexLinearCombination(l, inds.ptr(), weights.ptr());
  std::cout << "LOC:\n";
  std::cout << EXPR_AND_VAL_AS_STRING(inds) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(weights) << std::endl;
}

MDArray2d TargetSpeedParam::samplePolarCurve2d(Arrayd vertices,
    Velocity<double> windSpeed, int sampleCount) const {
  LineKM angle(0, sampleCount-1, 0.0, 360.0);
  MDArray2d xy(sampleCount, 2);
  for (int i = 0; i < sampleCount; i++) {
    auto alpha = Angle<double>::degrees(angle(i));

    auto boatSpeed = interpolate(alpha, windSpeed, vertices);
    auto motion = HorizontalMotion<double>::polar(windSpeed, alpha);
    //auto boatSpeed2 = interpolate(-alpha, windSpeed, vertices);
    auto boatSpeed2 = interpolate(calcLoc(motion), vertices);
    auto err = std::abs(boatSpeed - boatSpeed2);
    if (err > 1.0e-6) {
      auto alpha2 = motion.angle(); // + Angle<double>::degrees(360);
      auto alpha3 = motion.angle();
      auto windSpeed2 = motion.norm();
      std::cout << "--------------ERROR!!!!" << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(windSpeed) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(windSpeed2) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(alpha - alpha2) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(windSpeed - windSpeed2) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(_totalAngleCount) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(_totalRadiusCount) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(calcVertexIndex(2, 0)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(calcVertexIndex(2, 1)) << std::endl;
      dispLoc(*this, alpha, windSpeed);
      dispLoc(*this, alpha2, windSpeed2);
      /*std::cout << EXPR_AND_VAL_AS_STRING(windSpeed2 - motion.norm()) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(boatSpeed) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(boatSpeed2) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(windSpeed) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(alpha) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(alpha2) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(alpha - alpha2) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(interpolate(alpha2, windSpeed, vertices)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(interpolate(alpha3, windSpeed, vertices)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(motion.angle()) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(motion.norm()) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(interpolate(motion.angle(), motion.norm(), vertices)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(interpolate(alpha2, motion.norm(), vertices)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(interpolate(alpha2, windSpeed2, vertices)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(interpolate(calcLoc(motion), vertices)) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(err) << std::endl;*/
      assert(false);

    }

    auto pt = HorizontalMotion<double>::polar(
        Velocity<double>::knots(boatSpeed),
        alpha);
    xy(i, 0) = pt[0].knots();
    xy(i, 1) = pt[1].knots();
  }
  return xy;
}

MDArray2d TargetSpeedParam::samplePolarCurve3d(Arrayd vertices,
    Velocity<double> windSpeed, int sampleCount) const {
  LineKM angle(0, sampleCount-1, 0.0, 360.0);
  MDArray2d xyz(sampleCount, 3);
  for (int i = 0; i < sampleCount; i++) {
    auto alpha = Angle<double>::degrees(angle(i));
    auto motion = HorizontalMotion<double>::polar(windSpeed, alpha);
    xyz(i, 0) = motion[0].knots();
    xyz(i, 1) = motion[1].knots();

    auto z = interpolate(calcLoc(alpha, windSpeed), vertices);
    auto z2 = interpolate(calcLoc(-alpha, windSpeed), vertices);
    auto err = std::abs(z - z2);
    if (err > 1.0e-6) {
      std::cout << EXPR_AND_VAL_AS_STRING(err) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(z) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(z2) << std::endl;
    }

    xyz(i, 2) = z;
  }
  return xyz;
}

void TargetSpeedParam::calcVertexLinearCombination(Loc l,
    int *outInds, double *outWeights) const {
    int angleIndex = int(floor(l.angleIndex));
    double angleFrac = l.angleIndex - angleIndex;
    int radiusIndex = int(floor(l.radiusIndex));
    double radiusFrac = l.radiusIndex - radiusIndex;
    outInds[0] = calcVertexIndex(angleIndex + 0, radiusIndex + 0);
    outInds[1] = calcVertexIndex(angleIndex + 0, radiusIndex + 1);
    outInds[2] = calcVertexIndex(angleIndex + 1, radiusIndex + 0);
    outInds[3] = calcVertexIndex(angleIndex + 1, radiusIndex + 1);
    outWeights[0] = (1.0 - angleFrac)*(1.0 - radiusFrac);
    outWeights[1] = (1.0 - angleFrac)*radiusFrac;
    outWeights[2] = angleFrac*(1.0 - radiusFrac);
    outWeights[3] = angleFrac*radiusFrac;
}

Array<MDArray2d> TargetSpeedParam::samplePolarCurves(Arrayd vertices, bool dims3) const {
  int count = _totalRadiusCount - 2;
  Array<MDArray2d> curves(count);
  for (int i = 0; i < count; i++) {
    auto windSpeed = radiusIndexToWindSpeed(double(i + 1));
    curves[i] = (dims3? samplePolarCurve3d(vertices, windSpeed) : samplePolarCurve2d(vertices, windSpeed));
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
      auto bs = _param.interpolate(angle, speed, _vertices);
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

Array<MDArray2d> TargetSpeedFunction::samplePolarCurves(bool dims3,
    Velocity<double> unit) const {
    return _param.samplePolarCurves(
        _vertices.map<double>([&](Velocity<double> x) {
          return x/unit;
        }), dims3);
}


void TargetSpeedFunction::plotPolarCurves(
    bool dims3, Velocity<double> unit) const {
  auto curves = samplePolarCurves(dims3, unit);
  GnuplotExtra plot;
  plot.set_style("lines");
  for (int i = 0; i < curves.size(); i++) {
    plot.plot(curves[i]);
  }
  plot.show();
}









}
