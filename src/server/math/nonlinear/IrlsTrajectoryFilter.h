/*
 * IrlsTrajectoryFilter.h
 *
 *  Created on: Jun 9, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_
#define SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_

#include <server/common/AbstractArray.h>
#include <server/common/ArrayBuilder.h>
#include <server/math/irls.h>
#include <server/math/irlsFixedDenseBlock.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/TimedValue.h>

namespace sail {
namespace IrlsTrajectoryFilter {

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
    regWeight = 1.0;
  }
  double regWeight;
  irls::Settings irlsSettings;
  Velocity<double> maxSpeed;
};

/*
  for (int i = 0; i < N; ++i) {
    residuals[i] = _a*Xa[i] + _b*Xb[i] - T(_obs(i));
  }
  return true;
 */
template <int N>
irls::FixedDenseBlock<1, 2, N> *makeOrder0Block(
    int row, int col,
    double lambda, const Eigen::Matrix<double, N, 1> &obs) {
  typedef irls::FixedDenseBlock<1, 2, N> DstType;
  typename DstType::AType A;
  A(0, 0) = 1.0 - lambda;
  A(0, 1) = lambda;
  typename DstType::BType B = obs.transpose();
  return new DstType(A, B, row, col);
}


/*
    for (int i = 0; i < N; ++i) {
      residuals[i] = _diff(i) - (b[i] - a[i]);
    }
 */
template <int N>
irls::FixedDenseBlock<1, 2, N> *makeOrder1Block(
    int row, int col, const Eigen::Matrix<double, N, 1> &obs) {
  typedef irls::FixedDenseBlock<1, 2, N> DstType;
  typename DstType::AType A;
  A(0, 0) = -1.0;
  A(0, 1) = 1.0;
  typename DstType::BType B = obs.transpose();
  return new DstType(A, B, row, col);
}


/*
  Regularization(double weight, double balance) : _aw(weight*(1.0 - balance)),
    _bw(weight*balance), _weight(weight) {}

  template <typename T>
  bool operator()(const T* const a, const T* const b, const T* const c, T* residuals) const {
    for (int i = 0; i < N; i++) {
      residuals[i] = _weight*b[i] - (_aw*a[i] + _bw*c[i]);
    }
    return true;
  }
 */
template <int N>
irls::FixedDenseBlock<1, 3, N> *makeRegularization(int row, int col,
    double weight, double balance) {
  typedef irls::FixedDenseBlock<1, 3, N> DstType;
  double aw = weight*(1.0 - balance);
  double bw = weight*balance;
  typename DstType::AType A;
  A(0, 0) = aw;
  A(0, 1) = weight;
  A(0, 2) = bw;
  return new DstType(A, typename DstType::BType::Zero(), row, col);
}

template <int N>
typename Types<N>::Vector unwrap(const typename Types<N>::Position &x) {
  typename Types<N>::Vector dst;
  for (int i = 0; i < N; i++) {
    dst(i) = x[i]/getLengthUnit();
  }
  return dst;
}

template <int N>
typename Types<N>::TimedPositionArray filter(
    const AbstractArray<sail::TimeStamp> &samples,
    const AbstractArray<typename Types<N>::TimedPosition> &positions,
    const AbstractArray<typename Types<N>::TimedMotion> &motions,
    const Settings &settings) {

  typedef typename Types<N>::TimedPositionArray TimedPositionArray;

  if (!std::is_sorted(samples.begin(), samples.end())) {
    LOG(ERROR) << "Samples are not sorted";
    return TimedPositionArray();
  }

  LOG(INFO) << "Perform IrlsTrajectoryFilter";
  if (positions.empty()) {
    LOG(ERROR) << "At least one position is needed in order to perform the filtering";
    return TimedPositionArray();
  }

  int sampleCount = samples.size();

  if (sampleCount < 2) {
    LOG(ERROR) << "Needs at least two samples in order to filter (so that there is at least one interval)";
    return TimedPositionArray();
  }

  using namespace irls;
  ArrayBuilder<DenseBlock::Ptr> blocks;
  for (auto position: positions) {
    int intervalIndex = findIntervalWithTolerance(samples, position.time);
    if (intervalIndex != -1) {
      auto t0 = samples[intervalIndex];
      auto t1 = samples[intervalIndex+1];
      double lambda = computeLambda(t0, t1, position.time);
      blocks.add(makeOrder0Block<N>(blocks.size(), intervalIndex,
          lambda, unwrap<N>(position.value)));
    }
  }
  for (auto motion: motions) {
    int intervalIndex = findIntervalWithTolerance(samples, motion.time);
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

#endif /* SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_ */
