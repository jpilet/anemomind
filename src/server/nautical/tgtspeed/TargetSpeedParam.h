/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_POLARGRIDPARAM_H_
#define SERVER_NAUTICAL_TGTSPEED_POLARGRIDPARAM_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/math/Grid.h>
#include <server/common/string.h>
#include <server/common/math.h>
#include <type_traits>
#include <armadillo>

namespace sail {

// Parameterization of target speed function.
class TargetSpeedParam {
 public:
  TargetSpeedParam(bool symmetric, bool squaredRadius,
      int totalAngleCount, int totalRadiusCount,
      Velocity<double> maxWindVelocity);

  template <typename T>
  Velocity<T> radiusIndexToWindSpeed(T index) const {
    static_assert(!std::is_integral<T>::value, "Must not be integral");
    return _windFactor*distortRadius(index);
  }

  template <typename T>
  T windSpeedToRadiusIndex(Velocity<T> windSpeed) const {
    if (0 <= windSpeed.knots() && windSpeed < _maxWindSpeed) {
      return undistortRadius(windSpeed/_windFactor);
    }
    return T(-1);
  }

  template <typename T>
  Angle<T> angleIndexToWindAngle(T index) const {
    static_assert(!std::is_integral<T>::value, "Must not be integral");
    return _angleFactor*index;
  }

  template <typename T>
  T windAngleToAngleIndex(Angle<T> windAngle) const {
    return positiveMod<T>(windAngle/_angleFactor, _totalAngleCount);
  }

  template <typename T>
  T distortRadius(T x) const {
    return x*(_squaredRadius? x : 1.0);
  }

  template <typename T>
  T undistortRadius(T x) const {
    return (_squaredRadius? sqrt(x) : x);
  }

  int angleCellCount() const {
    return _totalAngleCount;
  }

  int radiusCellCount() const {
    return _totalRadiusCount - 1;
  }

  int totalCellCount() const {
    return angleCellCount()*radiusCellCount();
  }

  int calcCellIndex(int angleIndex, int radiusIndex) const {
    return angleIndex + angleCellCount()*radiusIndex;
  }

  int angleParamCount() const;
  int radiusParamCount() const;

  int paramCount() const {
    return angleParamCount()*radiusParamCount();
  }

  int vertexCount() const {
    return _totalAngleCount*_totalRadiusCount;
  }

  int calcRadiusParamIndex(int radiusIndex) const;
  int calcAngleParamIndex(int angleIndex) const;

  struct Loc {
   double angleIndex, radiusIndex;
   int cellIndex;

   bool valid() const {
     return 0 <= angleIndex && 0 <= radiusIndex;
   }
  };

  Loc calcLoc(Angle<double> windAngle, Velocity<double> windSpeed) const {
    double speedIndex = windSpeedToRadiusIndex(windSpeed);
    if (speedIndex < 0) {
      return Loc{-1, -1, -1};
    }
    double angleIndex = windAngleToAngleIndex(windAngle);
    return Loc{angleIndex, speedIndex, calcCellIndex(int(angleIndex), int(speedIndex))};
  }

  Loc calcLoc(HorizontalMotion<double> motion) const {
    return calcLoc(motion.angle(), motion.norm());
  }

  void calcVertexLinearCombination(Loc l, int *outInds, double *outWeights) const;

  template <typename T>
  T interpolate(const Loc &loc, Array<T> vertices) const {
    int inds[4] = {-1, -1, -1, -1};
    double weights[4] = {0, 0, 0, 0};
    calcVertexLinearCombination(loc, inds, weights);
    auto result = weights[0]*vertices[inds[0]];
    for (int i = 1; i < 4; i++) {
      result = result + weights[i]*vertices[inds[i]];
    }
    return result;
  }

  template <typename T>
  T interpolate(Angle<double> angle, Velocity<double> speed,
      Array<T> vertices) const {
    return interpolate(calcLoc(angle, speed), vertices);
  }

  int calcVertexIndex(int angleIndex, int radiusIndex) const;
  int calcParamIndex(int angleParamIndex, int radiusParamIndex) const;

  template <typename T>
  T lookUpParam(int index, Array<T> params) const {
    if (index == -1) {
      return T(0.0);
    } else {
      return params[index];
    }
  }



  template <typename T>
  T lookUpVertex(Array<T> vertices, int angleIndex, int radiusIndex) const {
    return vertices[calcVertexIndex(angleIndex, radiusIndex)];
  }

  template <typename T>
  Velocity<T> lookUpBoatSpeed(Array<T> vertices, int angleIndex, int radiusIndex) const {
    return Velocity<double>::knots(lookUpVertex(vertices, angleIndex, radiusIndex));
  }

  Array<HorizontalMotion<double> > vertexMotions() const;

  int radialRegCount() const {
    return std::max(0, (_totalRadiusCount - 2)*(_totalAngleCount - 1));
  }

  // Give lower weight for variations
  double normalizeRadial(int index) const {
    auto a = radiusIndexToWindSpeed(double(index)).knots();
    auto b = radiusIndexToWindSpeed(double(index-1)).knots();
    return 1.0/(a - b);
  }

  int angularRegCount() const {
    return (_totalRadiusCount - 1)*(_totalAngleCount - 1);
  }

  struct SubReg {
    Arrayi inds;
    arma::mat A;
    SubReg nextOrder() const;
    SubReg nextOrder(int steps) const;
  };
  Array<SubReg> makeRadialSubRegs() const;
  Array<SubReg> makeAngularSubRegs() const;

  arma::mat assembleReg(Array<SubReg> regs, int order) const;


  MDArray2d samplePolarCurve2d(Arrayd vertices,
      Velocity<double> windSpeed, int sampleCount = 120) const;

  MDArray2d samplePolarCurve3d(Arrayd vertices,
      Velocity<double> windSpeed, int sampleCount = 120) const;

  Array<MDArray2d> samplePolarCurves(Arrayd vertices, bool dims3) const;

  Array<MatrixElementd> makeNonNegativeVertexParam() const;
  MDArray2d makeNonNegativeVertexParamMatrix() const;
  Arrayd initializeNonNegativeParams() const;

  TargetSpeedParam();
 private:
  int _totalAngleCount, _totalRadiusCount;
  Velocity<double> _maxWindSpeed, _windFactor;
  Angle<double> _angleFactor;
  bool _symmetric, _squaredRadius;
};

// Represents a function that maps wind speed and angle to target speed.
class TargetSpeedFunction {
 public:
  TargetSpeedFunction() {}
  TargetSpeedFunction(TargetSpeedParam param,
      Array<Velocity<double> > vertices);
  bool defined() const;
  const Array<Velocity<double> > &vertices() const {return _vertices;}
  const TargetSpeedParam &param() const {return _param;}

  void outputTextTable(Array<Angle<double> > angles,
      Array<Velocity<double> > velocities, std::ostream *out) const;

  void outputNorthSailsTable(std::ostream *out) const;

  Array<MDArray2d> samplePolarCurves(bool dims3,
      Velocity<double> unit = Velocity<double>::knots(1.0)) const;

  void plotPolarCurves(
      bool dims3, Velocity<double> unit
        = Velocity<double>::knots(1.0)) const;
 private:
  TargetSpeedParam _param;
  Array<Velocity<double> > _vertices;
};

}

#endif /* SERVER_NAUTICAL_TGTSPEED_POLARGRIDPARAM_H_ */
