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
#include <server/common/IntervalUtils.h>

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
typename Types<N>::Vector mulAndUnwrap(
    const Duration<double> &dur, const typename Types<N>::Motion &m) {
  typename Types<N>::Vector dst;
  for (int i = 0; i < N; i++) {
    dst(i) = (dur*m[i])/getLengthUnit();
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
  typedef typename Types<N>::TimedPosition TimedPosition;

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
  ArrayBuilder<WeightingStrategy::Ptr> strategies;

  // Position fitness
  for (auto position: positions) {
    int intervalIndex = findIntervalWithTolerance(samples, position.time);
    if (intervalIndex != -1) {
      auto t0 = samples[intervalIndex];
      auto t1 = samples[intervalIndex+1];
      double lambda = computeLambda(t0, t1, position.time);
      int row = blocks.size();
      blocks.add(makeOrder0Block<N>(row, intervalIndex,
          lambda, unwrap<N>(position.value)));
      strategies.add(WeightingStrategy::Ptr(new Norm1Strategy(Spani(row, row+1))));
    }
  }

  // Motion fitness
  for (auto motion: motions) {
    int intervalIndex = findIntervalWithTolerance(samples, motion.time);
    if (intervalIndex != -1) {
      auto dur = samples[intervalIndex+1] - samples[intervalIndex];
      auto vec = mulAndUnwrap<N>(dur, motion.value);
      int row = blocks.size();
      blocks.add(makeOrder1Block<N>(row, intervalIndex, vec));
      strategies.add(WeightingStrategy::Ptr(new Norm1Strategy(Spani(row, row+1))));
    }
  }

  // Regularization
  LOG(INFO) << "Number of regularization terms: " << sampleCount - 2;
  for (int i = 1; i < sampleCount-1; i++) {
    double lambda = computeLambda<sail::TimeStamp>(samples[i-1], samples[i+1], samples[i]);
    blocks.add(makeRegularization<N>(blocks.size(), i-1, settings.regWeight, lambda));
  }

  auto allBlocks = blocks.get();


  auto results = irls::solveBanded(
      allBlocks.last()->requiredRows(),
      sampleCount, // TODO: Consider changing it later to N*sampleCount
      allBlocks, strategies.get(), settings.irlsSettings);

  if (results.X.rows() != sampleCount) {
    TimedPositionArray();
  }
  TimedPositionArray dst(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    TimedPosition p;
    p.time = samples[i];
    for (int j = 0; j < N; j++) {
      p.value[j] = results.X(i, j)*getLengthUnit();
    }
  }
  return dst;
}



}
}

#endif /* SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_ */
