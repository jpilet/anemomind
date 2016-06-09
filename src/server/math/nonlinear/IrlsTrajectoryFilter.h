/*
 * IrlsTrajectoryFilter.h
 *
 *  Created on: Jun 9, 2016
 *      Author: jonas
 */

#ifndef SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_
#define SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_

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
irls::FixedDenseBlock<1, 2, N> makeOrder0Block(
    int row, int col,
    double lambda, const Eigen::Matrix<double, N, 1> &obs) {
  typedef irls::FixedDenseBlock<1, 2, N> DstType;
  DstType::AType A;
  A(0, 0) = 1.0 - lambda;
  A(0, 1) = lambda;
  DstType::BType B = obs.transpose();
  return DstType(A, B, row, col);
}


/*
    for (int i = 0; i < N; ++i) {
      residuals[i] = _diff(i) - (b[i] - a[i]);
    }
 */
template <int N>
irls::FixedDenseBlock<1, 2, N> makeOrder1Block(
    int row, int col, const Eigen::Matrix<double, N, 1> &obs) {
  typedef irls::FixedDenseBlock<1, 2, N> DstType;
  DstType::AType A;
  A(0, 0) = -1.0;
  A(0, 1) = 1.0;
  DstType::BType B = obs.transpose();
  return DstType(A, B, row, col);
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
irls::FixedDenseBlock<1, 3, N> makeRegularization(int row, int col,
    double weight, double balance) {
  typedef irls::FixedDenseBlock<1, 3, N> DstType;
  double aw = weight*(1.0 - balance);
  double bw = weight*balance;
  DstType::AType A;
  A(0, 0) = aw;
  A(0, 1) = weight;
  A(0, 2) = bw;
  DstType::BType B = B::Zero();
  return DstType(A, B, row, col);
}




}
}

#endif /* SERVER_MATH_NONLINEAR_IRLSTRAJECTORYFILTER_H_ */
