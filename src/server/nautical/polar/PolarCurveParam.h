/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARCURVEPARAM_H_
#define POLARCURVEPARAM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/MDArray.h>
#include <server/common/LineKM.h>
#include <armadillo>

namespace sail {

/*
 * A class to parameterize polar curves, approximated by
 * line segments.
 */
class PolarCurveParam {
 public:
  PolarCurveParam();
  PolarCurveParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored);

  int ctrlToVertexIndex(int ctrlIndex) const {
    return _segsPerCtrlSpan*(1 + ctrlIndex);
  }

  int vertexCount() const {
    return lastVertex() + 1;
  }


  int paramCount() const {
    return _paramCount;
  }

  // Returns the dimension of the vertex vector
  int vertexDim() const {
    return 2*vertexCount(); // Every vertex has a 2d coordinate.
  }

  // Returns the dimension of the parameter vector
  int paramDim() const {
    return paramCount();
  }


  /*
   * Map a parameter vector to
   * a vertex vector.
   */
  template <typename T>
  void paramToVertices(Array<T> src,
      Array<T> dst /*destination should be preallocated
      and of the right size, for safety*/) const {
    assert(src.size() == paramDim());
    assert(dst.size() == vertexDim());

    arma::Mat<T> srcMat(src.getData(), src.size(), 1, false, true);
    arma::Mat<T> dstMat(dst.getData(), dst.size(), 1, false, true);
    dstMat = _Pmat*srcMat;

    // Make sure armadillo didn't reallocate anything.
    assert(dstMat.memptr() == dst.getData());
  }

  /*
   * Map a vertex vector and a position along the curve to a
   * position
   */
  template <typename T>
  void computeCurvePos(Array<T> vertexVector, double curvePos, T *dst2) const {
    assert(vertexVector.size() == vertexDim());
    assert(0 <= curvePos);
    assert(curvePos < 1.0);
    double vertexPos = _curveParamToVertexIndex(curvePos);
    int lowerIndex = int(floor(vertexPos));
    double lambda = vertexPos - lowerIndex;
    double lowerFactor = 1.0 - lambda;
    double upperFactor = lambda;
    T *lowerVertex = vertexVector.blockPtr(lowerIndex + 0, 2);
    T *upperVertex = vertexVector.blockPtr(lowerIndex + 1, 2);
    for (int i = 0; i < 2; i++) {
      dst2[i] = lowerFactor*lowerVertex[i] + upperFactor*upperVertex[i];
    }
  }

  void initializeParameters(Arrayd dst) const;
  Arrayd makeInitialParameters() const;

  int ctrlToParamIndex(int paramIndex) const;

  MDArray2d makePlotData(Arrayd params, double z = NAN) const;
 private:
  LineKM _curveParamToVertexIndex;
  Angle<double> ctrlAngle(int ctrlIndex) const;

  arma::mat _Pmat;
  int _segsPerCtrlSpan, _ctrlCount, _paramCount;
  bool _mirrored;

  int lastVertex() const {
    return ctrlToVertexIndex(_ctrlCount);
  }

  Arrayi makeAllCtrlInds() const;
  arma::mat makeP2CMat() const;
  arma::mat makeParamMat() const;
};

} /* namespace mmm */

#endif /* POLARCURVEPARAM_H_ */
