/*
 * SplineUtils.cpp
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#include <server/math/SplineUtils.h>
#include <server/math/lapack/BandWrappers.h>

namespace sail {

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
  std::cout << "SAMPLE COUNT = " << sampleCount << std::endl;
  int n = src.size();
  for (int k = 0; k < n; k++) {
    double x = toLocal(src[k]);
    double y = dst[k];
    std::cout << "Build weights" << std::endl;
    auto w = _basis.build(x);
    std::cout << "Weights built" << std::endl;
    for (int r = 0; r < w.dim; r++) {
      std::cout << "   i=" << w.inds[r] << " w=" << w.weights[r] << std::endl;

    }
    for (int i = 0; i < w.dim; i++) {
      for (int j = 0; j < w.dim; j++) {
        std::cout << "Inc lhs" << std::endl;
        int I = w.inds[i];
        int J = w.inds[j];
        lhs.add(I, J, w.weights[i]*w.weights[j]);
        std::cout << "Done\n";
      }
      std::cout << "Inc rhs" << std::endl;
      rhs(w.inds[i], 0) += w.weights[i]*y;
      std::cout << "Done" << std::endl;
    }
  }
  std::cout << "SOLVE IT" << std::endl;
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
