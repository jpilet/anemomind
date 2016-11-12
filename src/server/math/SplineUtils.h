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
#include <server/math/OutlierRejector.h>

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

Array<double> makePowers(int n, double f);

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
  Array<Basis> _bases;
  Array<double> _factors;
  SymmetricBandMatrixL<double> _A;
  MDArray2d _B;
  void initialize(int n, int dim, double k);
};

MDArray2d computeSplineCoefs(const MDArray2d &splineSamples);

template <int Dims>
class RobustSplineFit {
public:
  typedef SmoothBoundarySplineBasis<double, 3> Basis;
  typedef Basis::Weights Weights;
  typedef Eigen::Matrix<double, Dims, 1> Vec;
  typedef std::function<Vec(Vec)> VecFun;

  struct Observation {
    OutlierRejector rejector;
    double weight = 1.0;
    Weights param;
    Vec dst = Vec::Zero();
    Vec dstFun;
  };

  struct Settings {
    double minWeight = 0.1;
    double maxWeight = 10000.0;
    int iters = 8;

    int regOrder = 2;
    double regWeight = 1.0;

    int wellPosednessOrder = 1;
    double wellPosednessReg = 1.0e-5;
  };

  RobustSplineFit(const TimeMapper &mapper);

  void addObservation(TimeStamp t,
      int order,
      const Vec &value,
      double sigma,
      const VecFun &valueFun = VecFun());

  Array<Arrayd> solve();
private:
  std::vector<Observation> _observations;
  Array<Basis> _bases;
  Array<double> _factors;
  Settings _settings;
};

}

#endif /* SERVER_MATH_SPLINEUTILS_H_ */
