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
#include <server/common/Array.h>
#include <server/math/nonlinear/TimedObservation.h>

namespace sail {
namespace CeresTrajectoryFilter {

inline double computeLambda(double lower, double upper, double x) {
  return lower < upper?
    (x - lower)/(upper - lower) : 0.5;
}

struct Settings {
  Settings() {
    ceresOptions.linear_solver_type = ceres::SPARSE_NORMAL_CHOLESKY;
    regWeight = 1.0;
    huberThreshold = 10.0;
  }
  double regWeight;
  double huberThreshold;
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
class Order1Fit {
public:
  Order1Fit(
      double scale,
      const Eigen::Matrix<double, N, 1> &obs) : _scale(scale), _diff(obs) {}

  template <typename T>
  bool operator()(const T* const a, const T* const b,
                  T* residuals) const {
    for (int i = 0; i < N; ++i) {
      residuals[i] = _diff(i) - _scale*(b[i] - a[i]);
    }
    return true;
  }

  static ceres::CostFunction* Create(double scale, const Eigen::Matrix<double, N, 1> &obs) {
    return (new ceres::AutoDiffCostFunction<Order1Fit<N>, N, N, N>
            (new Order1Fit<N>(scale, obs)));
  }
private:
  double _scale;
  Eigen::Matrix<double, N, 1> _diff;
};

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
bool areGood(const Array<TimedObservation<N> > &observations) {
  if (!std::is_sorted(observations.begin(), observations.end())) {
    LOG(ERROR) << "Observations are not sorted";
    return false;
  }
  bool hasOrder0 = false;
  for (auto obs: observations) {
    if (obs.order == 0) {
      hasOrder0 = true;
    }
  }
  if (!hasOrder0) {
    LOG(ERROR) << "There are no position observations, so the problem is ill-posed";
    return false;
  }
  return true;
}


template <int N>
bool addObservation(
    ceres::Problem *problem,
    ceres::LossFunction *loss,
    int intervalIndex,
    const Arrayd &samples,
    Array<Eigen::Matrix<double, N, 1> > &X,
    const TimedObservation<N> &obs) {
  double left = samples[intervalIndex];
  double right = samples[intervalIndex+1];
  double lambda = computeLambda(left, right, obs.time);
  auto Xa = X[intervalIndex].data();
  auto Xb = X[intervalIndex+1].data();

  double marg = 1.0e-3;
  if (lambda < 0.0 - marg || 1.0 + marg < lambda) {
    return true; // Don't treat this as an error.
  }

  if (obs.order == 0) {
    problem->AddResidualBlock(Order0Fit<N>::Create(lambda, obs.value),
        loss, Xa, Xb);
    return true;
  } else if (obs.order == 1) {
    auto t = right - left;
    problem->AddResidualBlock(Order1Fit<N>::Create(1.0, t*obs.value),
                  loss, Xa, Xb);
    return true;
  } else {
    LOG(ERROR) << "Don't know what to do with observation of order " << obs.order << std::endl;
    return false;
  }
}


template <int N>
Array<Eigen::Matrix<double, N, 1> > filter(
    const Arrayd &samples,
    const Array<TimedObservation<N> > &observations,
    const Settings &settings,
    Array<Eigen::Matrix<double, N, 1> > Xinit) {

  if (!std::is_sorted(samples.begin(), samples.end())) {
    LOG(ERROR) << "Samples are not sorted";
    return Array<Eigen::Matrix<double, N, 1> >();
  }

  LOG(INFO) << "Perform CeresTrajectoryFilter";

  if (!areGood(observations)) {
    LOG(ERROR) << "The observations are bad";
    return Array<Eigen::Matrix<double, N, 1> >();
  }

  int sampleCount = samples.size();

  if (sampleCount < 2) {
    LOG(ERROR) << "Needs at least two samples in order to filter (so that there is at least one interval)";
    return Array<Eigen::Matrix<double, N, 1> >();
  }

  int n = observations.size();


  CHECK(Xinit.empty() || Xinit.size() == sampleCount);
  Array<Eigen::Matrix<double, N, 1> > X = Xinit.empty()?
      Array<Eigen::Matrix<double, N, 1> >::fill(
          sampleCount, Eigen::Matrix<double, N, 1>::Zero())
          : Xinit;

  ceres::Problem problem;

  int intervalIndex = 0;

  // Observations
  LOG(INFO) << "Number of observations: " << observations.size();
  auto loss = new ceres::HuberLoss(settings.huberThreshold);
  for (int i = 0; i < n; i++) {
    auto obs = observations[i];
    intervalIndex = moveIntervalIndexForward(samples, intervalIndex, obs.time);
    if (!addObservation(&problem, loss, intervalIndex, samples, X, obs)) {
      LOG(ERROR) << "Bad observation";
      return Array<Eigen::Matrix<double, N, 1> >();
    }
  }

  // Regularization
  LOG(INFO) << "Number of regularization terms: " << n - 2;
  for (int i = 1; i < sampleCount-1; i++) {
    double lambda = computeLambda(samples[i-1], samples[i+1], samples[i]);
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

    return Array<Eigen::Matrix<double, N, 1> >();
  }
  LOG(INFO) << "Successfully performed CeresTrajectoryFilter.";
  return X;
}

}
}

#endif /* SERVER_MATH_NONLINEAR_CERESTRAJECTORYFILTER_H_ */
