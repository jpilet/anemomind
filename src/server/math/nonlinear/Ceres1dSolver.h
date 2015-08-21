/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_
#define SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_

#include <server/math/nonlinear/BandedSolver.h>
#include <ceres/ceres.h>
#include <server/common/ArrayIO.h>
#include <server/common/logging.h>

namespace sail {
namespace Ceres1dSolver {

struct Settings {
  BandedSolver::Settings commonSettings;
  enum LossType {L1, L2};

  LossType dataLoss = L1;
  LossType regLoss = L1;

  ceres::LinearSolverType solverType = ceres::DENSE_NORMAL_CHOLESKY; //ceres::SPARSE_NORMAL_CHOLESKY;

  /*SPARSE_NORMAL_CHOLESKY,
  SPARSE_SCHUR,
  ITERATIVE_SCHUR*/
};

ceres::LossFunction *makeLossFunction(Settings::LossType t, double lb) {
  switch (t) {
   case Settings::L1:
     return new ceres::SoftLOneLoss(lb);
   case Settings::L2:
     return new ceres::TrivialLoss();
  };
  return nullptr;
}

template <typename T>
T softSqrt(T x, T lb) {
  if (x <= T(0)) {
    return T(0);
  } else if (x < lb) {
    T sqrtLb = sqrt(lb);
    return sqrtLb - 0.5*x/sqrtLb;
  }
  return x;
}

template <typename T>
MDArray<T, 2> makeMat(int rows, int cols, const T *src) {
  MDArray<T, 2> data(rows, cols);
  int n = data.numel();
  T *dst = data.ptr();
  for (int i = 0; i < n; i++) {
    dst[i] = src[i];
  }
  return data;
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

    eval<T>(makeMat(_sampling.count(), Dim, x[0]), residual);
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
      auto r2 = obs.calcSquaredResidualT(X);
      residuals[i] = softSqrt(r2, T(_settings.commonSettings.residualLowerBound));
    }
  }
};

template <int Dim>
class RegCost {
 public:
  RegCost(Sampling sampling, Settings settings) :
    _sampling(sampling),
    _settings(settings) {}

  template<typename T>
  bool operator()(const T* const *x, T* residual) {
    eval<T>(makeMat(_sampling.count(), Dim, x[0]), residual);
    return true;
  }

  int outDims() const {
    return _sampling.count() - _settings.commonSettings.regOrder;
  }

  int inDims() const {
    return Dim*_sampling.count();
  }
 private:
  Sampling _sampling;
  Settings _settings;

  template <typename T>
  void eval(MDArray<T, 2> X, T *residuals) const {
    auto difs = BandedSolver::calcDifsInPlace<T, Dim>(
        _settings.commonSettings.regOrder, X);
    int rows = difs.rows();
    CHECK(rows == outDims());
    for (int i = 0; i < rows; i++) {
      T r2(0);
      for (int j = 0; j < Dim; j++) {
        r2 += sqr(difs(i, j));
      }
      residuals[i] = softSqrt(_settings.commonSettings.lambda*r2,
          T(_settings.commonSettings.residualLowerBound));
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

  CHECK(X.isContinuous());

  ceres::Problem problem;
  {
    ceres::DynamicAutoDiffCostFunction<DataCost<Dim> > *dataCost = makeCeresCost(new DataCost<Dim>(sampling, observations, settings));
    problem.AddResidualBlock(dataCost,
        makeLossFunction(settings.dataLoss,
        settings.commonSettings.residualLowerBound),
        X.ptr());
  }{
    auto regCost = makeCeresCost(new RegCost<Dim>(sampling, settings));
    problem.AddResidualBlock(regCost,
        makeLossFunction(settings.regLoss,
        settings.commonSettings.residualLowerBound),
        X.ptr());

  }
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = settings.commonSettings.iters;
  options.linear_solver_type = settings.solverType;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;
  return X;
}


};
}

#endif /* SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_ */
