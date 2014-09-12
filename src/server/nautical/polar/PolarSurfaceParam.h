/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARSURFACEPARAM_H_
#define POLARSURFACEPARAM_H_

#include <server/nautical/polar/PolarCurveParam.h>

namespace sail {

class PolarSurfaceParam {
 public:

  PolarSurfaceParam();
  PolarSurfaceParam(PolarCurveParam pcp,
      Velocity<double> maxTws,
      int twsLevelCount);

  int paramCount() const {
    return _twsLevelCount*_polarCurveParam.paramCount();
  }

  int vertexCount() const {
    return _twsLevelCount*_polarCurveParam.vertexCount();
  }

  int vertexDim() const { // Only the 2d positions of the vertices
    return 2*vertexCount();
  }

  /*
   * This method maps a parameter vector to a vector
   * of vertex coordinates, in knots. It is knot intended
   * to be used directly, but as an input to a mapping function
   * 'computeSurfacePoint'.
   */
  template <typename T>
  void paramToVertices(Array<T> params, Array<T> verticesOut) const {
    assert(params.size() == paramCount());
    assert(verticesOut.size() == vertexDim());
    Array<T> actualCurveParams = Array<T>::fill(_polarCurveParam.paramDim(), T(0));
    for (int i = 0; i < _twsLevelCount; i++) {

      /* This curious mapping is to ensure that the parameters for one level
       * are greater than those of the level below, and that they are positive.*/
      Array<T> pi = curveParams(i, params);
      for (int j = 0; j < _polarCurveParam.paramDim(); j++) {
        actualCurveParams[j] = exp(*pi[j]) + actualCurveParams[j];
      }

      _polarCurveParam.paramToVertices(actualCurveParams,
          curveVertices(i, verticesOut));
    }
  }

  /*
   *
   */
  template <typename T>
  void computeSurfacePoint(Array<T> vertices, const double *surfaceCoord2,
    Velocity<T> *outXYZ) {

  }

  Arrayd makeInitialParams() const;
 private:
  template <typename T>
  Array<T> curveVertices(int levelIndex, Array<T> vertices) const {
    return vertices.sliceBlock(levelIndex, _polarCurveParam.vertexDim());
  }

  template <typename T>
  Array<T> curveParams(int levelIndex, Array<T> params) const {
    return params.sliceBlock(levelIndex, _polarCurveParam.paramDim());
  }

  PolarCurveParam _polarCurveParam;
  Velocity<double> _twsStep;
  int _twsLevelCount;
};

}

#endif /* POLARSURFACEPARAM_H_ */
