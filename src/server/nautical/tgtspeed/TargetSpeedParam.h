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

// Parameterization of a function that maps TWA and TWS to boat speed.
// This function is parameterized using a set of non-negative parameters,
// that map to so called vertices. The vertices are boat speeds at different grid
// points in a polar coordinate system. An actual function, for a particular
// set of parameters, is a TargetSpeedFunction.
class TargetSpeedParam {
 public:
  TargetSpeedParam(bool symmetric, bool squaredRadius,
      int totalAngleCount, int totalRadiusCount,
      Velocity<double> maxWindVelocity);

  // Indexing of radii in the grid. Indices are from 0 to _totalRadiusCount-1.
  // This function converts an index to a corresponding radius in the grid.
  // See also angleIndexToWindAngle for the analogous function in angular direction.
  template <typename T>
  Velocity<T> radiusIndexToWindSpeed(T index) const {
    static_assert(!std::is_integral<T>::value, "Must not be integral");
    return _windFactor*distortRadius(index);
  }

  template <typename T>
  T windSpeedToRadiusIndex(Velocity<T> windSpeed) const {
    if (0 <= windSpeed.knots() && windSpeed < _maxWindSpeed) {
      return undistortRadius<T>(windSpeed/_windFactor);
    }
    return T(-1);
  }

  // Indexing of angles in the grid. Indices are from 0 to _totalAngleCount-1.
  // This function converts an index to a corresponding angle.
  // See also radiusIndexToWindIndex for the analogous function in the radial direction.
  template <typename T>
  Angle<T> angleIndexToWindAngle(T index) const {
    static_assert(!std::is_integral<T>::value, "Must not be integral");
    return _angleFactor*index;
  }

  template <typename T>
  T windAngleToAngleIndex(Angle<T> windAngle) const {
    auto frac = windAngle/_angleFactor;
    if (std::isnan(frac)) {
      return frac;
    }
    return positiveMod<T>(frac, _totalAngleCount);
  }

  template <typename T>
  T distortRadius(T x) const {
    return _squaredRadius? x*x : x;
  }

  template <typename T>
  T undistortRadius(T x) const {
    return (_squaredRadius? T(sqrt(x)) : T(x));
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
    if (0 <= angleIndex && angleIndex < _totalAngleCount &&
        0 <= radiusIndex && radiusIndex < _totalRadiusCount) {
      return angleIndex + angleCellCount()*radiusIndex;
    }
    return -1;
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

  // Used for the same purpose as barycentric coordinates
  // to encode features on a 3d surface to be reconstructed.
  // In our case, each face on the surface has four corners,
  // so using bilinear interpolation is natural. We use this
  // to encode the TWA and TWS from TargetSpeedPoint. Then we
  // use the computed indices and weights in order to interpolate
  // a boat speed.
  struct BilinearWeights {
   BilinearWeights();
   int cellIndex;

   int inds[4];
   double weights[4];


   bool valid() const {
     return 0 <= cellIndex;
   }

   template <typename T>
   T eval(const Array<T> &X) const {
     return weights[0]*X[inds[0]] + weights[1]*X[inds[1]] +
         weights[2]*X[inds[2]] + weights[3]*X[inds[3]];
   }
  };

  BilinearWeights calcBilinearWeights(Angle<double> windAngle, Velocity<double> windSpeed) const;

  BilinearWeights calcBilinearWeights(HorizontalMotion<double> motion) const {
    return calcBilinearWeights(motion.angle(), motion.norm());
  }

  int calcVertexLinearCombination(double angleIndex, double radiusIndex,
      int *outInds, double *outWeights) const;

  int calcVertexLinearCombination(Angle<double> angle, Velocity<double> radius,
      int *outInds, double *outWeights) const;

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




  Array<MatrixElementd> makeNonNegativeVertexParam() const;
  MDArray2d makeNonNegativeVertexParamMatrix() const;
  Arrayd initializeNonNegativeParams() const;

  TargetSpeedParam();

  int totalAngleCount() const {
    return _totalAngleCount;
  }

  int totalRadiusCount() const {
    return _totalRadiusCount;
  }
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

  Array<MDArray2d> samplePolarCurves(bool dims3) const;

  void plotPolarCurves(
      bool dims3, Velocity<double> unit
        = Velocity<double>::knots(1.0)) const;


  MDArray2d samplePolarCurve2d(
      Velocity<double> windSpeed, int sampleCount = 60) const;

  MDArray2d samplePolarCurve3d(
      Velocity<double> windSpeed, int sampleCount = 60) const;

  Velocity<double> calcBoatSpeed(Angle<double> windAngle, Velocity<double> windSpeed) const;
 private:
  TargetSpeedParam _param;
  Array<Velocity<double> > _vertices;
};

}

#endif /* SERVER_NAUTICAL_TGTSPEED_POLARGRIDPARAM_H_ */
