/*
 * SplineUtils.h
 *
 *  Created on: 28 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_SPLINEUTILS_H_
#define SERVER_MATH_SPLINEUTILS_H_

#include <server/math/Spline.h>
#include <server/common/TimeMapper.h>
#include <server/math/lapack/BandMatrix.h>

namespace sail {

std::ostream &operator<<(std::ostream &s,
    SmoothBoundarySplineBasis<double, 3>::Weights w);

Arrayd fitSplineCoefs(
    const SmoothBoundarySplineBasis<double, 3> &basis,
    std::function<double(int)> sampleFun);

class TemporalSplineCurve {
public:
  TemporalSplineCurve(
      Span<TimeStamp> timeSpan,
      Duration<double> period,
      Array<TimeStamp> src,
      Array<double> dst);

  double evaluate(TimeStamp t) const;
private:
  double toLocal(TimeStamp x) const;
  TimeStamp _offset;
  Duration<double> _period;
  SmoothBoundarySplineBasis<double, 3> _basis;
  Array<double> _coefs;
};

class SplineFittingProblem {
public:
  typedef SmoothBoundarySplineBasis<double, 3> Basis;

  SplineFittingProblem(
      const TimeMapper &mapper,
      int dim);

  SplineFittingProblem(int n, int dim);

  void addCost(
      int order,
      double weight,
      double x, double *y);
  void addCost(
      int order,
      double weight,
      TimeStamp t, double *y);

  void addRegularization(int order, double weight);

  MDArray2d solve();

  Basis basis(int i = 0) const;

  void disp() const;
private:
  static const int N = 4;
  TimeMapper _mapper;
  Basis _bases[N];
  double _factors[N];

  SymmetricBandMatrixL<double> _A;
  MDArray2d _B;

  void initialize(int n, int dim, double k);
};

MDArray2d computeSplineCoefs(const MDArray2d &splineSamples);

}

#endif /* SERVER_MATH_SPLINEUTILS_H_ */
