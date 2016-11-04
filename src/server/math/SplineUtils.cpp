/*
 * SplineUtils.cpp
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#include <server/math/SplineUtils.h>
#include <server/math/lapack/BandWrappers.h>

namespace sail {

void accumulateNormalEqs(
    const SmoothBoundarySplineBasis<double, 3>::Weights &w, double y,
    SymmetricBandMatrixL<double> *lhs,
    MDArray2d *rhs) {
  for (int i = 0; i < w.dim; i++) {
    if (w.isSet(i)) {
      for (int j = 0; j < w.dim; j++) {
        if (w.isSet(j)) {
          int I = w.inds[i];
          int J = w.inds[j];
          lhs->add(I, J, w.weights[i]*w.weights[j]);
        }
      }
      (*rhs)(w.inds[i], 0) += w.weights[i]*y;
    }
  }
}

Arrayd fitSplineCoefs(
    const SmoothBoundarySplineBasis<double, 3> &basis,
    std::function<double(int)> sampleFun) {
  int n = basis.coefCount();
  auto lhs = SymmetricBandMatrixL<double>::zero(
      n, SmoothBoundarySplineBasis<double, 3>::Weights::dim);
  auto rhs = MDArray2d(n, 1);
  std::cout << "Solving for " << n << std::endl;
  for (int i = 0; i < n; i++) {
    accumulateNormalEqs(basis.build(i), sampleFun(i), &lhs, &rhs);
  }
  std::cout << "NOw solv it" << std::endl;
  Pbsv<double>::apply(&lhs, &rhs);
  return rhs.getStorage();
}

TemporalSplineCurve::TemporalSplineCurve(
    Span<TimeStamp> timeSpan,
    Duration<double> period,
    Array<TimeStamp> src,
    Array<double> dst) :
        _offset(timeSpan.minv()), _period(period) {
  CHECK(src.size() == dst.size());
  int sampleCount = int(ceil(toLocal(timeSpan.maxv())));
  _basis = SmoothBoundarySplineBasis<double, 3>(sampleCount);

  auto lhs = SymmetricBandMatrixL<double>::zero(
      sampleCount, decltype(_basis)::Weights::dim);

  auto rhs = MDArray2d(sampleCount, 1);
  rhs.setAll(0.0);
  for (int i = 0; i < sampleCount; i++) {
    lhs.add(i, i, 1.0e-12);
  }
  int n = src.size();
  for (int k = 0; k < n; k++) {
    double x = toLocal(src[k]);
    double y = dst[k];
    auto w = _basis.build(x);
    accumulateNormalEqs(w, y, &lhs, &rhs);
  }
  Pbsv<double>::apply(&lhs, &rhs);
  _coefs = rhs.getStorage();
}

double TemporalSplineCurve::toLocal(TimeStamp x) const {
  return (x - _offset)/_period;
}

double TemporalSplineCurve::evaluate(TimeStamp t) const {
  return _basis.evaluate(_coefs.ptr(), toLocal(t));
}

} /* namespace sail */
