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
#include <server/math/nonlinear/CeresTrajectoryFilter.h>

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
    regWeight = 10.0;
    inlierThreshold = Length<double>::meters(12.0);
    maxSpeed = Velocity<double>::knots(100.0);
  }
  double regWeight;
  irls::Settings irlsSettings;
  Velocity<double> maxSpeed;
  Length<double> inlierThreshold;
};

/*
  for (int i = 0; i < N; ++i) {
    residuals[i] = _a*Xa[i] + _b*Xb[i] - T(_obs(i));
  }
  return true;
 */
template <int N>
irls::DenseBlock::Ptr makeOrder0Block(
    int row, int col,
    double lambda, const Eigen::Matrix<double, N, 1> &obs) {
  typedef irls::FixedDenseBlock<N, 2*N, 1> DstType;
  typename DstType::AType A = Eigen::Matrix<double, N, 2*N>::Zero();
  double a = 1.0 - lambda;
  double b = lambda;
  for (int i = 0; i < N; i++) {
    A(i, i) = a;
    A(i, i + N) = b;
  }
  return irls::DenseBlock::Ptr(new DstType(A, obs, row, col));
}


/*
    for (int i = 0; i < N; ++i) {
      residuals[i] = _diff(i) - (b[i] - a[i]);
    }
 */
template <int N>
irls::DenseBlock::Ptr makeOrder1Block(
    int row, int col, const Eigen::Matrix<double, N, 1> &obs) {
  typedef irls::FixedDenseBlock<N, 2*N, 1> DstType;
  typename DstType::AType A = Eigen::Matrix<double, N, 2*N>::Zero();
  for (int i = 0; i < N; i++) {
    A(i, i) = -1.0;
    A(i, i + N) = 1.0;
  }
  return irls::DenseBlock::Ptr(new DstType(A, obs, row, col));
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
irls::DenseBlock::Ptr makeRegularization(int row, int col,
    double weight, double balance) {
  typedef irls::FixedDenseBlock<N, 3*N, 1> DstType;
  double aw = weight*(1.0 - balance);
  double bw = weight*balance;
  typename DstType::AType A = Eigen::Matrix<double, N, 3*N>::Zero();
  for (int i = 0; i < N; i++) {
    A(i, i) = aw;
    A(i, N + i) = -weight;
    A(i, 2*N + i) = bw;
  }
  return irls::DenseBlock::Ptr(
      new DstType(A,
          Eigen::Matrix<double, N, 1>::Zero(), row, col));
}

template <int N>
irls::DenseBlock::Ptr makeStepSizeBlock(
    int row, int col) {
  return makeOrder1Block<N>(row, col, Eigen::Matrix<double, N, 1>::Zero());
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

  double tau = settings.inlierThreshold/getLengthUnit();

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

  auto currentRow = [&]() {
    return N*blocks.size();
  };

  // Position fitness
  for (auto position: positions) {
    int intervalIndex = CeresTrajectoryFilter::findIntervalWithTolerance(samples, position.time);
    if (intervalIndex != -1) {
      auto t0 = samples[intervalIndex];
      auto t1 = samples[intervalIndex+1];
      double lambda = computeLambda(t0, t1, position.time);
      int row = currentRow();
      blocks.add(makeOrder0Block<N>(row, N*intervalIndex,
          lambda, unwrap<N>(position.value)));
      strategies.add(WeightingStrategy::Ptr(new Norm1Strategy(Spani(row, row+N), 1.0, tau)));
    }
  }

  // Motion fitness
  for (auto motion: motions) {
    int intervalIndex = CeresTrajectoryFilter::findIntervalWithTolerance(samples, motion.time);
    if (intervalIndex != -1) {
      auto dur = samples[intervalIndex+1] - samples[intervalIndex];
      auto vec = mulAndUnwrap<N>(dur, motion.value);
      int row = currentRow();
      blocks.add(makeOrder1Block<N>(row, N*intervalIndex, vec));
      strategies.add(WeightingStrategy::Ptr(new Norm1Strategy(Spani(row, row+N), 1.0, tau)));
    }
  }

  // Regularization
  LOG(INFO) << "Number of regularization terms: " << sampleCount - 2;
  for (int i = 1; i < sampleCount-1; i++) {
    double lambda = computeLambda<sail::TimeStamp>(samples[i-1], samples[i+1], samples[i]);
    blocks.add(makeRegularization<N>(currentRow(), N*(i-1), settings.regWeight, lambda));
  }

  // Step size
  LOG(INFO) << "Bounded steps";
  for (int i = 0; i < sampleCount-1; i++) {
    auto dur = samples[i+1] - samples[i];
    Length<double> maxStep = dur*settings.maxSpeed;
    int row = currentRow();
    blocks.add(makeStepSizeBlock<N>(row, N*i));
    strategies.add(WeightingStrategy::Ptr(new BoundedNormConstraint(Spani(row, row + N),
        maxStep/getLengthUnit())));
  }

  auto allBlocks = blocks.get();

  int rowCount = allBlocks.last()->requiredRows();
  assert(rowCount == currentRow());
  auto results = irls::solveBanded(
      rowCount,
      N*sampleCount,
      allBlocks, strategies.get(), settings.irlsSettings);

  if (results.X.rows() != N*sampleCount) {
    TimedPositionArray();
  }
  TimedPositionArray dst(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    int offset = N*i;
    TimedPosition p;
    p.time = samples[i];
    for (int j = 0; j < N; j++) {
      p.value[j] = results.X(offset + j)*getLengthUnit();
    }
    dst[i] = p;
  }
  return dst;
}



}
}

#endif /* SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_ */
