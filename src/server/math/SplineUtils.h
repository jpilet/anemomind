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

template <typename T>
struct TimedValue;

typedef SmoothBoundarySplineBasis<double, 3> CubicBasis;

std::ostream &operator<<(std::ostream &s,
    SmoothBoundarySplineBasis<double, 3>::Weights w);

Arrayd fitSplineCoefs(
    const SmoothBoundarySplineBasis<double, 3> &basis,
    std::function<double(int)> sampleFun);


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
  double weight = 10;
  int order = 2;
  double wellPosednessReg = 1.0e-5;
};

template <int N>
using VecObs = std::pair<double, Eigen::Matrix<double, N, 1>>;

struct AutoRegResults {
  Array<double> costPerIterationHistory;
  MDArray2d coefs;
  bool defined() const {return !coefs.empty();}
};

template <int N>
AutoRegResults fitSplineAutoReg(
    int coefCount,
    const Array<VecObs<N>>
      &observations,
      const AutoRegSettings &settings,
      std::default_random_engine *rng);

struct UnitVecSplineOp {
  typedef Eigen::Vector2d Vec;
  typedef Vec OutputType;
  static const int coefDim = 2;
  Vec apply(Vec v) const {
    auto len = v.norm();
    return 0 < len? Vec((1.0/len)*v) : Vec(0, 0);
  }
};

template <typename OpType>
class TypedSpline {
public:
  TypedSpline(
        const TimeMapper &mapper,
        const MDArray2d &coefs,
        OpType op);
  TypedSpline() {}

  TimeStamp lower() const;
  TimeStamp upper() const;
  typename OpType::OutputType
    evaluate(const TimeStamp &t) const;
private:
  OpType _op;
  TimeMapper _timeMapper;
  CubicBasis _basis;
  MDArray2d _coefs;
};

TypedSpline<UnitVecSplineOp>
  fitAngleSpline(
      const TimeMapper &mapper,
      const Array<TimedValue<Angle<double>>> &angles,
      const AutoRegSettings &settings,
      std::default_random_engine *rng);

}

#endif /* SERVER_MATH_SPLINEUTILS_H_ */
