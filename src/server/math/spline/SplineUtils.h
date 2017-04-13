/*
 * SplineUtils.h
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_SPLINEUTILS_H_
#define SERVER_MATH_SPLINEUTILS_H_

#include <server/math/spline/Spline.h>
#include <server/common/TimeMapper.h>

namespace sail {

template <typename T>
struct TimedValue;

template <typename BASIS>
struct CoefsWithOffset {
  static constexpr int dim = BASIS::Weights::dim;
  int offset;
  Eigen::Matrix<double, 1, dim> coefs;

  CoefsWithOffset() : offset(0), coefs(Eigen::Matrix<double, 1, dim>::Zero()) { }

  CoefsWithOffset(const typename BASIS::Weights& w) {
    for (int i = 0; i < w.dim; i++) {
      if (w.isSet(i)) {
        offset = w.inds[i];
        break;
      }
    }

    for (int i = 0; i < w.dim; i++) {
      if (w.isSet(i)) {
        coefs(0, w.inds[i] - offset) += w.weights[i];
      }
    }
  }
};

typedef SmoothBoundarySplineBasis<double, 3> CubicBasis;

class SplineCurve {
public:
  SplineCurve() {}

  SplineCurve(
      const CubicBasis& basis,
      const MDArray<double, 2>& coefs) :
        _basis(basis), _coefs(coefs) {}

  double evaluate(int dim, double x) const {
    auto weights = _basis.build(x);
    return weights.evaluate(_coefs.getPtrAt(0, dim));
  }

  template <int N>
  Eigen::Matrix<double, N, 1> evaluate(double x) const {
    assert(N == _coefs.cols());
    auto weights = _basis.build(x);
    Eigen::Matrix<double, N, 1> dst;
    for (int i = 0; i < N; i++) {
      dst(i) = weights.evaluate(_coefs.getPtrAt(0, i));
    }
    return dst;
  }

  SplineCurve derivative() const {
    return SplineCurve(_basis.derivative(), _coefs);
  }

  Array<SplineCurve> derivatives() const {
    auto d = _basis.derivatives();
    int n = d.size();
    Array<SplineCurve> dst(n);
    for (int i = 0; i < n; i++) {
      dst[i] = SplineCurve(d[i], _coefs);
    }
    return dst;
  }

  bool empty() const {return _coefs.empty();}

  const CubicBasis& basis() const {return _basis;}
private:
  CubicBasis _basis;
  MDArray2d _coefs;
};

Array<double> makePowers(int n, double f);

template <typename T>
class PhysicalTemporalSplineCurve {
public:
  PhysicalTemporalSplineCurve() {}
  PhysicalTemporalSplineCurve(
      const TimeMapper& m,
      const SplineCurve& c,
      T u, const Span<int>& isp = Span<int>()) : _mapper(m), _curve(c), _unit(u),
          _indexSpan(isp.initialized()? isp :
              Span<int>(0, m.sampleCount())) {}

  PhysicalTemporalSplineCurve<TimeDerivative<T>> derivative() const {
    return PhysicalTemporalSplineCurve<TimeDerivative<T>>(
        _mapper, _curve.derivative(), _unit/_mapper.period(),
        _indexSpan);
  }

  T evaluate(int dim, TimeStamp t) const {
    return _unit*_curve.evaluate(dim, _mapper.toRealIndex(t));
  }

  bool empty() const {return _curve.empty();}

  TimeMapper timeMapper() const {
    return _mapper;
  }

  T unit() const {return _unit;}

  Span<int> indexSpan() const {
    return _indexSpan;
  }
private:
  TimeMapper _mapper;
  SplineCurve _curve;
  T _unit;
  Span<int> _indexSpan;
};

}

#endif /* SERVER_MATH_SPLINEUTILS_H_ */
