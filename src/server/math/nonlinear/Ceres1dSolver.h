/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_
#define SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_

#include <server/math/nonlinear/BandedSolver.h>
#include <ceres/ceres.h>

namespace sail {
namespace Ceres1dSolver {

struct Settings {
  BandedSolver::Settings commonSettings;
  enum LossType {L1};

  LossType dataLoss = L1;
  LossType regLoss = L1;

  ceres::LinearSolverType solverType = ceres::SPARSE_NORMAL_CHOLESKY;

  /*SPARSE_NORMAL_CHOLESKY,
  SPARSE_SCHUR,
  ITERATIVE_SCHUR*/
};

ceres::LossFunction *makeLossFunction(Settings::LossType t, double lb) {
  switch (t) {
   case Settings::L1:
     return new ceres::SoftLOneLoss(lb);
  };
  return nullptr;
}


template <int Dim>
class DataCost {
 public:
  DataCost(const Sampling &sampling,
      Array<Observation<Dim> > observations,
      Settings settings) :
        _sampling(sampling),
        _observations(observations),
        _settings(settings) {}

  template<typename T>
      bool operator()(const T* const *x, T* residual) {
    MDArray<T, 2> data(_sampling.count(), Dim);
    int n = data.numel();
    T *dst = data.ptr();
    const T *src = x[0];
    for (int i = 0; i < n; i++) {
      dst[i] = src[i];
    }
    eval<T>(data, residual);
    return true;
  }

  int outDims() const {
    return _observations.size();
  }

  int inDims() const {
    return Dim*_sampling.count();
  }
 private:
  Sampling _sampling;
  Array<Observation<Dim> > _observations;
  Settings _settings;

  template <typename T>
  void eval(const MDArray<T, 2> &X, T *residuals) const {
    for (int i = 0; i < _observations.size(); i++) {
      const auto &obs = _observations[i];
      auto r = obs.calcResidualT(X);
      std::cout << EXPR_AND_VAL_AS_STRING(r) << std::endl;
      residuals[i] = r;
    }
  }
};



template <typename T>
ceres::DynamicAutoDiffCostFunction<T> *makeCeresCost(T *objf) {
  auto cost = new ceres::DynamicAutoDiffCostFunction<T>(objf);
  cost->AddParameterBlock(objf->inDims());
  cost->SetNumResiduals(objf->outDims());
  return cost;
}

template <int Dim>
MDArray2d solve(Sampling sampling,
    Array<Observation<Dim> > observations, Settings settings,
    MDArray2d initialX = MDArray2d()) {
  Arrayd regCoefs = BandMatInternal::makeCoefs(settings.commonSettings.regOrder);
  MDArray2d X = (initialX.empty()?
      BandedSolver::initialize(sampling.count(), Dim) : initialX);
  const int paramCount = Dim*sampling.count();

  assert(X.isContinuous());

  ceres::Problem problem;
  {
    ceres::DynamicAutoDiffCostFunction<DataCost<Dim> > *dataCost = makeCeresCost(new DataCost<Dim>(sampling, observations, settings));
    problem.AddResidualBlock(dataCost,
        makeLossFunction(settings.dataLoss,
        settings.commonSettings.residualLowerBound),
        X.ptr());
  }/*{
    auto regCost = makeCeresCost(new RegCost<Dim>(sampling, observations, settings));
    problem.AddResidualBlock(regCost,
        makeLossFunction(settings.regLoss,
        settings.commonSettings.residualLowerBound),
        X.ptr());

  }*/
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = settings.commonSettings.iters;
  options.linear_solver_type = settings.solverType;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  return X;
}


};
}

#endif /* SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_ */
