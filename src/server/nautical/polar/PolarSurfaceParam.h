/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARSURFACEPARAM_H_
#define POLARSURFACEPARAM_H_

#include <server/nautical/polar/PolarCurveParam.h>
#include <adolc/adouble.h>
#include <server/common/ToDouble.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>

namespace sail {

/*
 * This function maps a real number
 * (negative or positive) to a positive number.
 *
 * It has a more suitable behaviour than exp in the
 * sense that it doesn't grow exponentially for large numbers,
 * but linearly.
 */
template <typename T>
T expline(T x) {
  if (ToDouble(x) < 0) {
    return exp(x);
  } else {
    return 1.0 + exp(1.0)*x;
  }
}

// inverse function to the function above.
template <typename T>
T logline(T x) {
  if (ToDouble(x) > 1.0) {
    return (x - 1.0)*exp(-1.0);
  } else {
    return log(x);
  }
}

template <typename T>
MDArray<T, 2> mulTransposed(MDArray<double, 2> A, MDArray<T, 2> B) {
  int rows = A.rows();
  int cols = B.cols();
  int middle = A.cols();
  assert(middle == B.rows());
  MDArray<T, 2> dst(cols, rows); // <-- transpose the result
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      T sum = T(0);
      for (int k = 0; k < middle; k++) {
        sum += A(i, k)*B(k, j);
      }
      dst(j, i) = sum;
    }
  }
  return dst;
}

class GnuplotExtra;
class PolarSurfaceParam {
 public:

  PolarSurfaceParam();
  PolarSurfaceParam(PolarCurveParam pcp,
      Velocity<double> maxTws,
      int twsLevelCount, int ctrlCount=-1);

  int paramCount() const {
    return _ctrlCount*_polarCurveParam.paramCount();
  }

  int vertexCount() const {
    return _twsLevelCount*_polarCurveParam.vertexCount();
  }

  int vertexDim() const { // Only the 2d positions of the vertices
    return 2*vertexCount();
  }

  /*
   * This method maps a parameter vector to a vector
   * of vertex coordinates, in knots. It is not intended
   * to be used directly, but as an input to a mapping function
   * 'computeSurfacePoint'.
   */
  template <typename T>
  void allParamToVertices(Array<T> params, Array<T> verticesOut) const {
    assert(params.size() == _twsLevelCount*_polarCurveParam.paramDim());
    assert(verticesOut.size() == vertexDim());
    Array<T> actualCurveParams = Array<T>::fill(_polarCurveParam.paramDim(), T(0));

    for (int i = 0; i < _twsLevelCount; i++) {

      /* This curious mapping is to ensure that the parameters for one level
       * are greater than those of the level below, and that they are positive.*/
      Array<T> pi = curveParams(i, params);
      for (int j = 0; j < _polarCurveParam.paramDim(); j++) {
        actualCurveParams[j] += expline(pi[j]);
      }
      _polarCurveParam.paramToVertices(actualCurveParams,
          curveVertices(i, verticesOut));
    }
  }

  template <typename T>
  void paramToVertices(Array<T> params, Array<T> verticesOut) const {
    if (_P.empty()) {
      allParamToVertices(params, verticesOut);
    } else {
      Array<T> acc = Array<T>::fill(_polarCurveParam.paramDim(), T(0));

      // Build the ctrl matrix
      MDArray<T, 2> ctrlp(_ctrlCount, _polarCurveParam.paramDim());
      for (int i = 0; i < _ctrlCount; i++) {
        Array<T> pi = curveParams(i, params);
        for (int j = 0; j < _polarCurveParam.paramDim(); j++) {
          T &x = acc[j];
          x += expline(pi[j]);
          ctrlp(i, j) = x;
        }
      }

      assert(verticesOut.size() == vertexDim());
      assert(_P.isContinuous());
      MDArray<T, 2> dst = mulTransposed(_P, ctrlp);
      Array<T> allparams = dst.getStorage();
      for (int i = 0; i < _twsLevelCount; i++) {
        _polarCurveParam.paramToVertices(curveParams(i, allparams), curveVertices(i, verticesOut));
      }
    }
  }



  Arrayd makeInitialParams() const;

  template <typename T>
  Vectorize<Velocity<T>, 3> computeSurfacePoint(Array<T> vertices,
      Vectorize<double, 2> surfaceCoord2) const {
    Velocity<T> dst[3];
    computeSurfacePointSub(vertices, surfaceCoord2.data(), dst);
    return Vectorize<Velocity<T>, 3>(dst);
  }

  Velocity<double> targetSpeed(Arrayd vertices,
      Velocity<double> tws, Angle<double> twa) const;

  /*
   * Generate random points, uniformly distributed
   * on the surface.
   */
  static Array<Vectorize<double, 2> > generateSurfacePoints(int count);

  void plot(Arrayd paramsOrVertices, GnuplotExtra *dst) const;
  void plot(Arrayd paramsOrVertices) const;

  // Make vertex data
  MDArray2d makeVertexData(Arrayd paramsOrVertices) const;



  bool withCtrl() const {
    return !_P.empty();
  }
 private:

  Arrayd toVertices(Arrayd paramsOrVertices) const;

  template <typename T>
  void computeSurfacePointSub(Array<T> vertices,
      const double *surfaceCoord2,
      Velocity<T> *outXYZ3) const {
    assert(0 <= surfaceCoord2[0]); assert(surfaceCoord2[0] <= 1.0);
    assert(0 <= surfaceCoord2[1]); assert(surfaceCoord2[1] <= 1.0);

    outXYZ3[2] = T(surfaceCoord2[1])*_maxTws.cast<T>();

    double curvep = surfaceCoord2[0];
    double curveIndex = surfaceCoord2[1]*_twsLevelCount - 1;
    int lower = std::min(int(floor(curveIndex)), _twsLevelCount-1);
    int upper = lower + 1;
    double lambda = curveIndex - lower;
    double lowerWeight = 1.0 - lambda;
    double upperWeight = lambda;

    outXYZ3[0] = Velocity<double>::knots(0);
    outXYZ3[1] = Velocity<double>::knots(0);
    addWeightedCurveVertex(vertices, lower,
        lowerWeight, curvep, outXYZ3);
    addWeightedCurveVertex(vertices, upper,
        upperWeight, curvep, outXYZ3);
  }


  template <typename T>
  void addWeightedCurveVertex(Array<T> vertices,
      int index, double weight, double curvep,
      Velocity<T> *outXY) const {
    if (index >= 0) {
      Array<T> subv = curveVertices(index, vertices);
      T tmp[2];
      _polarCurveParam.computeCurvePos(subv, curvep, tmp);
      for (int i = 0; i < 2; i++) {
        outXY[i] += T(weight)*Velocity<T>::knots(tmp[i]);
      }
    }
  }

  template <typename T>
  Array<T> curveVertices(int levelIndex, Array<T> vertices) const {
    return vertices.sliceBlock(levelIndex, _polarCurveParam.vertexDim());
  }

  template <typename T>
  Array<T> curveParams(int levelIndex, Array<T> params) const {
    return params.sliceBlock(levelIndex, _polarCurveParam.paramDim());
  }

  PolarCurveParam _polarCurveParam;
  Velocity<double> _twsStep, _maxTws;
  int _twsLevelCount, _ctrlCount;

  // A linear parameterization of the polar slices, using a a few "control" slices.
  // This means that only a sparse set of slices a specified, and the remaining ones are interpolated.
  MDArray2d _P;

  Arrayi _ctrlInds;
};

}

#endif /* POLARSURFACEPARAM_H_ */
