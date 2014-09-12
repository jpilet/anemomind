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
  void computeSurfacePoint(Array<T> vertices,
      const double *surfaceCoord2,
      Velocity<T> *outXYZ3) {
    assert(0 <= surfaceCoord2[0]); assert(surfaceCoord2[0] <= 1.0);
    assert(0 <= surfaceCoord2[1]); assert(surfaceCoord2[1] <= 1.0);

    outXYZ3[2] = surfaceCoord2[1]*_twsStep.cast<T>();

    double curveVertexIndex = _polarCurveParam.toVertexIndex(surfaceCoord2[0]);
    double curveIndex = surfaceCoord2[1]*_twsLevelCount - 1;
    int lower = int(floor(curveIndex));
    int upper = lower + 1;
    double lambda = curveIndex - lower;
    double lowerWeight = 1.0 - lambda;
    double upperWeight = lambda;

    outXYZ3[0] = Velocity<double>::knots(0);
    outXYZ3[1] = Velocity<double>::knots(0);
    addWeightedCurveVertex(vertices, lower,
        lowerWeight, curveVertexIndex, outXYZ3);
    addWeightedCurveVertex(vertices, upper,
        upperWeight, curveVertexIndex, outXYZ3);
  }

  Arrayd makeInitialParams() const;


 private:
  template <typename T>
  void addWeightedCurveVertex(Array<T> vertices,
      int index, double weight, double curveVertexIndex,
      Velocity<T> *outXY) const {
    if (index >= 0) {
      Array<T> subv = curveVertices(index, vertices);
      T tmp[2];
      _polarCurveParam.computeCurvePos(subv, curveVertexIndex, tmp);
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
  int _twsLevelCount;
};

}

#endif /* POLARSURFACEPARAM_H_ */
