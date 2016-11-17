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
#include <random>

namespace sail {

typedef SmoothBoundarySplineBasis<double, 3> CubicBasis;

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
Eigen::Matrix<double, Dims, 1> evaluateSpline(
    const SmoothBoundarySplineBasis<double, 3>::Weights &weights,
        const MDArray2d &coefs);

template <int Dims>
class RobustSplineFit {
public:
  typedef SmoothBoundarySplineBasis<double, 3> Basis;
  typedef Basis::Weights Weights;
  typedef Eigen::Matrix<double, Dims, 1> Vec;
  typedef std::function<Vec(Vec)> VecFun;

  struct Observation {
    OutlierRejector rejector;
    Weights weights0, weights;
    double dstScale = 1.0;
    Vec dst = Vec::Zero();
    VecFun dstFun;

    Vec computeDst(const MDArray2d &coefs) const {
      return dstScale*((!coefs.empty() && bool(dstFun))?
        dstFun(evaluateSpline<Dims>(weights0, coefs))
            : dst);
    }

    double computeResidual(const MDArray2d &coefs) const {
      return (computeDst(coefs)
          - evaluateSpline<Dims>(weights, coefs)).norm();
    }

    Observation() {}
    Observation(const Weights &w0, const Weights &w,
        double ds, const Vec &v,
        const VecFun &vf, const OutlierRejector::Settings &os) :
          weights0(w0), weights(w), dstScale(ds),
            dst(v), dstFun(vf),
            rejector(os) {}
  };

  struct Settings {
    double minWeight = 0.1;
    double maxWeight = 10000.0;
    int iters = 8;

    int regOrder = 2;
    double regWeight = 1.0;

    int wellPosednessOrder = 1;
    double wellPosednessReg = 1.0e-5;

    bool ignoreConstantIfVariable = true;
  };

  RobustSplineFit(
      const TimeMapper &mapper,
      const Settings &settings = Settings());

  void addObservation(TimeStamp t,
      int order,
      const Vec &value,
      double sigma,
      const VecFun &valueFun = VecFun());

  MDArray2d solve();
  Basis basis(int i = 0) const;
private:
  TimeMapper _mapper;
  std::vector<Observation> _observations;
  Array<Basis> _bases;
  Array<double> _factors;
  Settings _settings;
};

struct AutoRegSettings {
  int maxIters = 5;
  double weight = 1.0;
  int order = 2;
};

template <int N>
MDArray2d fitSplineAutoReg(
    int coefCount,
    const Array<std::pair<int, Eigen::Matrix<double, N, 1>>>
      &observations,
      const AutoRegSettings &settings,
      std::default_random_engine *rng);

}

#endif /* SERVER_MATH_SPLINEUTILS_H_ */
