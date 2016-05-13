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

template <int N>
class Order0Fit {
public:
  Order0Fit(const TimedObservation<N> &obs) : _obs(obs) {}

  template <typename T>
  bool operator()(const T* const point,
                  T* residuals) const {
    for (int i = 0; i < N; ++i) {
      residuals[i] = point[i] - T(_obs.value(i));
    }
    return true;
  }

  static ceres::CostFunction* Create(const TimedObservation<N> &obs) {
    return (new ceres::AutoDiffCostFunction<Order0Fit<N>, N, N>
            (new Order0Fit<N>(obs)));
  }
private:
  TimedObservation<N> _obs;
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
Array<Eigen::Matrix<double, N, 1> > filter(
    const Array<TimedObservation<N> > &observations,
    const Settings &settings,
    Array<Eigen::Matrix<double, N, 1> > Xinit) {

  LOG(INFO) << "Perform CeresTrajectoryFilter";

  if (!areGood(observations)) {
    LOG(ERROR) << "The observations are bad";
    return Array<Eigen::Matrix<double, N, 1> >();
  }

  int n = observations.size();
  CHECK(Xinit.empty() || Xinit.size() == n);
  Array<Eigen::Matrix<double, N, 1> > X = Xinit.empty()?
      Array<Eigen::Matrix<double, N, 1> >::fill(
          n, Eigen::Matrix<double, N, 1>::Zero())
          : Xinit;

  ceres::Problem problem;

  // Observations
  LOG(INFO) << "Number of observations: " << observations.size();
  auto loss = new ceres::HuberLoss(settings.huberThreshold);
  for (int i = 0; i < n; i++) {
    auto obs = observations[i];
    if (obs.order == 0) {
      problem.AddResidualBlock(Order0Fit<N>::Create(obs),
          loss,
          X[i].data());
    } else if (obs.order == 1) {
      if (i == 0) {
        auto t = observations[1].time - obs.time;
        problem.AddResidualBlock(Order1Fit<N>::Create(1.0, t*obs.value),
            loss, X[0].data(), X[1].data());
      } else if (i == n-1) {
        auto t = obs.time - observations[i-1].time;
        problem.AddResidualBlock(Order1Fit<N>::Create(1.0, t*obs.value),
                    loss, X[i-1].data(), X[i].data());
      } else {
        auto t = 0.5*(observations[i+1].time - observations[i-1].time);
        problem.AddResidualBlock(Order1Fit<N>::Create(0.5, t*obs.value),
                    loss, X[i-1].data(), X[i+1].data());
      }
    } else {
      LOG(ERROR) << "Don't know what to do with observation of order " << obs.order << std::endl;
      return Array<Eigen::Matrix<double, N, 1> >();
    }
  }

  // Regularization
  LOG(INFO) << "Number of regularization terms: " << n - 2;
  for (int i = 1; i < n-1; i++) {
    double fullDiff = observations[i+1].time - observations[i-1].time;
    double partialDiff = observations[i].time - observations[i-1].time;
    double balance = (fullDiff <= 0.0? 0.5 : partialDiff/fullDiff);
    problem.AddResidualBlock(Regularization<N>::Create(settings.regWeight, balance), nullptr,
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
