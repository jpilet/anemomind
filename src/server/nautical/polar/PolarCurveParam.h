/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARCURVEPARAM_H_
#define POLARCURVEPARAM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/MDArray.h>
#include <armadillo>

namespace sail {

/*
 * A class to parameterize polar curves, approximated by
 * line segments.
 */
class PolarCurveParam {
 public:
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


  template <typename T>
  void paramToVertices(Array<T> src,
      Array<T> dst /*destination should be preallocated
      and of the right size, for safety*/) {
    assert(src.size() == paramDim());
    assert(dst.size() == vertexDim());

    arma::Mat<T> srcMat(src.getData(), src.size(), 1, false, true);
    arma::Mat<T> dstMat(dst.getData(), dst.size(), 1, false, true);
    dstMat = _Pmat*srcMat;

    // Make sure armadillo didn't reallocate anything.
    assert(dstMat.memptr() == dst.getData());
  }



  int ctrlToParamIndex(int paramIndex) const;
 private:
  Angle<double> ctrlAngle(int ctrlIndex) const;

  arma::mat _Pmat;
  int _segsPerCtrlSpan, _ctrlCount, _paramCount;
  bool _mirrored;

  int lastVertex() const {
    return ctrlToVertexIndex(_ctrlCount);
  }

  Arrayi makeCtrlInds() const;
  arma::mat makeP2CMat() const;
};

} /* namespace mmm */

#endif /* POLARCURVEPARAM_H_ */
