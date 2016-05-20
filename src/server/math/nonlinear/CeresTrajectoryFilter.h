/*
 * CeresTrajectoryFilter.h
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_CERESTRAJECTORYFILTER_H_
#define SERVER_MATH_NONLINEAR_CERESTRAJECTORYFILTER_H_

#include <ceres/ceres.h>
#include <ceres/types.h>
#include <server/common/TimedValue.h>
#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/IntervalUtils.h>
#include <server/common/AbstractArray.h>


namespace sail {
namespace CeresTrajectoryFilter {

// This is the unit used for the residuals in the optimization
inline Length<double> getLengthUnit() {
  return Length<double>::meters(1.0);
}


template <int N>
struct Types {

  typedef Vectorize<Length<double>, N> LengthVector;
  typedef LengthVector Position;
  typedef Vectorize<Velocity<double>, N> Motion;

  typedef Eigen::Matrix<double, N, 1> Vector;

  typedef TimedValue<Position> TimedPosition;
  typedef TimedValue<Motion> TimedMotion;

  typedef Array<TimedPosition> TimedPositionArray;
  typedef Array<TimedMotion> TimedMotionArray;
};

struct Settings {
  Settings() {
    ceresOptions.linear_solver_type = ceres::SPARSE_NORMAL_CHOLESKY;
    regWeight = 1.0;
    huberThreshold = Length<double>::meters(10.0);
  }
  double regWeight;
  Length<double> huberThreshold;
  ceres::Solver::Options ceresOptions;
};

int moveIntervalIndexForward(const Arrayd &samples, int intervalIndex, double t);

template <int N>
class Order0Fit {
public:
  Order0Fit(double lambda,
      const Eigen::Matrix<double, N, 1> &obs) :
        _a(1.0 - lambda), _b(lambda), _obs(obs) {}

  template <typename T>
  bool operator()(const T* const Xa, const T* const Xb,
                  T* residuals) const {
    for (int i = 0; i < N; ++i) {
      residuals[i] = _a*Xa[i] + _b*Xb[i] - T(_obs(i));
    }
    return true;
  }

  static ceres::CostFunction* Create(double lambda,
      const Eigen::Matrix<double, N, 1> &obs) {
    return (new ceres::AutoDiffCostFunction<Order0Fit<N>, N, N, N>
            (new Order0Fit<N>(lambda, obs)));
  }
private:
  double _a, _b;
  Eigen::Matrix<double, N, 1> _obs;
};

template <int N>
typename Types<N>::Vector unwrap(const typename Types<N>::Position &x) {
  typename Types<N>::Vector dst;
  for (int i = 0; i < N; i++) {
    dst(i) = x[i]/getLengthUnit();
  }
  return dst;
}

template <int N>
ceres::CostFunction *makePositionCost(TimeStamp a, TimeStamp b,
    typename Types<N>::TimedPosition &position) {
  double lambda = computeLambda(a, b, position.time);
  return Order0Fit<N>::Create(lambda, unwrap<N>(position.value));
}

template <int N>
class Order1Fit {
public:
  Order1Fit(const Eigen::Matrix<double, N, 1> &obs) : _diff(obs) {}

  template <typename T>
  bool operator()(const T* const a, const T* const b,
                  T* residuals) const {
    for (int i = 0; i < N; ++i) {
      residuals[i] = _diff(i) - (b[i] - a[i]);
    }
    return true;
  }

  static ceres::CostFunction* Create(const Eigen::Matrix<double, N, 1> &obs) {
    return (new ceres::AutoDiffCostFunction<Order1Fit<N>, N, N, N>
            (new Order1Fit<N>(obs)));
  }
private:
  Eigen::Matrix<double, N, 1> _diff;
};

template <int N>
typename Types<N>::Vector mulAndUnwrap(
    const Duration<double> &dur, const typename Types<N>::Motion &m) {
  typename Types<N>::Vector dst;
  for (int i = 0; i < N; i++) {
    dst(i) = Length<double>::meters(dur.seconds()*m[i].metersPerSecond())/getLengthUnit();
  }
  return dst;
}

template <int N>
class Regularization {
public:
  Regularization(double weight, double balance) : _aw(weight*(1.0 - balance)),
    _bw(weight*balance), _weight(weight) {}

  template <typename T>
  bool operator()(const T* const a, const T* const b, const T* const c, T* residuals) const {
    for (int i = 0; i < N; i++) {
      residuals[i] = _weight*b[i] - (_aw*a[i] + _bw*c[i]);
    }
    return true;
  }

  static ceres::CostFunction* Create(double weight, double balance) {
    return (new ceres::AutoDiffCostFunction<Regularization<N>, N, N, N, N>
            (new Regularization<N>(weight, balance)));
  }
private:
  double _weight, _aw, _bw;
};

template <int N>
ceres::CostFunction* makeMotionCost(
    Duration<double> dur, const typename Types<N>::TimedMotion &m) {
  return Order1Fit<N>::Create(mulAndUnwrap<N>(dur, m.value));
}

template <int N>
Array<typename Types<N>::Vector> initializeX(
    int sampleCount,
    const AbstractArray<typename Types<N>::Position> &initialPositions) {
  if (initialPositions.size() == 0) {
    return Array<typename Types<N>::Vector>::fill(
        sampleCount, Types<N>::Vector::Zero());
  } else {
    Array<typename Types<N>::Vector> dst(sampleCount);
    for (int i = 0; i < sampleCount; i++) {
      auto &y = dst[i];
      const auto &x = initialPositions[i];
      for (int j = 0; j < N; j++) {
        y[j] = x[j]/getLengthUnit();
      }
    }
    return dst;
  }
}

template <int N>
typename Types<N>::TimedPosition wrapResult(sail::TimeStamp t, const Eigen::Matrix<double, N, 1> &x) {
  typename Types<N>::TimedPosition dst;
  dst.time = t;
  for (int j = 0; j < N; j++) {
    dst.value[j] = x(j)*getLengthUnit();
  }
  return dst;
}

template <int N>
typename Types<N>::TimedPositionArray wrapResult(
    const AbstractArray<TimeStamp> &times, const Array<Eigen::Matrix<double, N, 1> > &X) {
  int n = times.size();
  if (n != X.size()) {
    LOG(ERROR) << "Incompatible sizes";
    return typename Types<N>::TimedPositionArray();
  }
  typename Types<N>::TimedPositionArray Y(n);
  for (int i = 0; i < n; i++) {
    Y[i] = wrapResult(times[i], X[i]);
  }
  return Y;
}

int findIntervalIndex2(const AbstractArray<TimeStamp> &times, TimeStamp t);

template <int N>
typename Types<N>::TimedPositionArray filter(
    const AbstractArray<sail::TimeStamp> &samples,
    const AbstractArray<typename Types<N>::TimedPosition> &positions,
    const AbstractArray<typename Types<N>::TimedMotion> &motions,
    const Settings &settings,
    const AbstractArray<typename Types<N>::Position> &initialPositions) {

  typedef typename Types<N>::TimedPositionArray TimedPositionArray;

  if (!std::is_sorted(samples.begin(), samples.end())) {
    LOG(ERROR) << "Samples are not sorted";
    return TimedPositionArray();
  }

  LOG(INFO) << "Perform CeresTrajectoryFilter";
  if (positions.empty()) {
    LOG(ERROR) << "At least one position is needed in order to perform the filtering";
    return TimedPositionArray();
  }

  int sampleCount = samples.size();

  if (sampleCount < 2) {
    LOG(ERROR) << "Needs at least two samples in order to filter (so that there is at least one interval)";
    return TimedPositionArray();
  }

  Array<typename Types<N>::Vector> X = initializeX<N>(
      sampleCount, initialPositions);

  ceres::Problem problem;

  ceres::LossFunction *loss = new ceres::HuberLoss(
      settings.huberThreshold/getLengthUnit());

  for (auto position: positions) {
    int intervalIndex = findIntervalIndex2(samples, position.time);
    if (intervalIndex != -1) {
      problem.AddResidualBlock(makePositionCost<N>(
          samples[intervalIndex], samples[intervalIndex+1], position),
          loss, X[intervalIndex].data(), X[intervalIndex+1].data());
    }
  }
  for (auto motion: motions) {
    int intervalIndex = findIntervalIndex2(samples, motion.time);
    if (intervalIndex != -1) {
      problem.AddResidualBlock(makeMotionCost<N>(
          samples[intervalIndex+1] - samples[intervalIndex], motion),
          loss, X[intervalIndex].data(), X[intervalIndex+1].data());
    }
  }

  // Regularization
  LOG(INFO) << "Number of regularization terms: " << sampleCount - 2;
  for (int i = 1; i < sampleCount-1; i++) {
    double lambda = computeLambda<sail::TimeStamp>(samples[i-1], samples[i+1], samples[i]);
    problem.AddResidualBlock(Regularization<N>::Create(settings.regWeight, lambda), nullptr,
        X[i-1].data(), X[i].data(), X[i+1].data());
  }

  ceres::Solver::Summary summary;
  LOG(INFO) << "Optimizing...";
  ceres::Solve(settings.ceresOptions, &problem, &summary);
  LOG(INFO) << "Done optimizing";

  // I don't consider reaching the maximum number of iterations (that is NO_CONVERGENCE)
  // a bad outcome. This is perfectly possible, and we probably want to use that result.
  std::set<ceres::TerminationType> badOutcomes{
    ceres::FAILURE, ceres::USER_FAILURE
  };

  if (badOutcomes.count(summary.termination_type) == 1) {
    LOG(ERROR) << "Ceres failed " << summary.FullReport();

    return TimedPositionArray();
  }
  LOG(INFO) << "Successfully performed CeresTrajectoryFilter.";
  return wrapResult<N>(samples, X);
}

}
}

#endif /* SERVER_MATH_NONLINEAR_CERESTRAJECTORYFILTER_H_ */
