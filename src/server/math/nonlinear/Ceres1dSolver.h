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
  enum LossType {
    L1, // Like AbsCost
    L2  // Like SquareCost
    };

  LossType dataLoss = L1;
  LossType regLoss = L1;

  ceres::LinearSolverType solverType = ceres::SPARSE_NORMAL_CHOLESKY;
};

inline ceres::LossFunction *makeLossFunction(Settings::LossType t, double lb) {
  switch (t) {
   case Settings::L1:
     return new ceres::SoftLOneLoss(lb);
   case Settings::L2:
     return new ceres::TrivialLoss();
  };
  return nullptr;
}


// Replace sqrt by this function, in order to avoid
// non-differentiability issues at 0.
template <typename T>
T softSqrt(T x, double lb = 0.1) {
  if (x < T(0)) {
    return softSqrt(-x, lb);
  } else if (x < lb) {
    auto f = sqrt(lb);
    auto df = 0.5/f;
    /*auto maj = MajQuad::majorize(lb, f, df);
    auto offset = f - maj.a*sqr(lb);
    return offset + maj.a*sqr(x);*/
    return f - (lb - x)*df;
  }
  return sqrt(x);
}


/*
 * This is the data fitness cost.
 */
template <int Dim>
class DataCost {
 public:
  DataCost(Observation<Dim> observation, double lb) :
        _observation(observation),
        _lb(lb) {}

  template<typename T>
      bool operator()(const T* x, const T* y, T* residual) const {
    T r2(0.0);
    auto w = _observation.weights;
    for (int i = 0; i < Dim; i++) {
      r2 += sqr(w.lowerWeight*x[i] + w.upperWeight*y[i] - _observation.data[i]);
    }
    auto dataResidual = softSqrt<T>(r2);
    residual[0] = dataResidual;
    return true;
  }
 private:
  Observation<Dim> _observation;
  double _lb;
};


/*
 * This is the regularization cost.
 */
template <int Dim>
class RegCost {
 public:
  RegCost(Settings settings) :
    _settings(settings) {}

  template<typename T>
  bool operator()(const T* const *x, T* residual) {
    static MDArray<T, 2> X;
    const int rows = _settings.commonSettings.regOrder + 1;
    const int cols = Dim;
    if (X.cols() != cols || X.rows() != rows) {
      X = MDArray<T, 2>(rows, cols);
    }
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        X(i, j) = x[i][j];
      }
    }

    auto difs = BandedSolver::calcDifsInPlace<T, Dim>(
        _settings.commonSettings.regOrder, X
    );
    assert(difs.rows() == 1);

    T r2(0.0);
    for (int j = 0; j < Dim; j++) {
      r2 += sqr(difs(0, j));
    }
    auto regResidual = _settings.commonSettings.lambda*softSqrt(r2);
    residual[0] = regResidual;
    return true;
  }

  int outDims() const {
    return 1;
  }

 private:
  Settings _settings;
};



/*
 * Creates a cost function for Ceres from an arbitrary object
 * with the operator() to eval, and the methods inDims and outDims.
 */
template <typename T>
ceres::DynamicAutoDiffCostFunction<T> *makeCeresRegCost(int Dim,
    T *objf, int paramBlockCount) {
  auto cost = new ceres::DynamicAutoDiffCostFunction<T>(objf);
  cost->SetNumResiduals(1);
  for (int i = 0; i < paramBlockCount; i++) {
    cost->AddParameterBlock(Dim);
  }
  return cost;
}

inline double *getBlockPtr(MDArray2d X, int index) {
  return X.sliceCol(index).ptr();
}

inline MDArray2d transpose(MDArray2d X) {
  int rows = X.rows();
  int cols = X.cols();
  MDArray2d Y(cols, rows);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Y(j, i) = X(i, j);
    }
  }
  return Y;
}

template <int Dim>
MDArray2d solve(Sampling sampling,
    Array<Observation<Dim> > observations, Settings settings,
    MDArray2d initialX = MDArray2d()) {
  Arrayd regCoefs = BandMatInternal::makeCoefs(settings.commonSettings.regOrder);
  MDArray2d X(Dim, sampling.count());
  for (int i = 0; i < X.rows(); i++) {
    for (int j = 0; j < X.cols(); j++) {
      X(i, j) = sin(3.324*i);
    }
  }
  CHECK(X.isContinuous());

  ceres::Problem problem;
  for (int i = 0; i < sampling.count(); i++) {
    problem.AddParameterBlock(X.sliceCol(i).ptr(), Dim);
  }

  int i = 0;
  for (Observation<Dim> obs: observations) {
    i++;
    auto loss = makeLossFunction(settings.dataLoss,
            settings.commonSettings.residualLowerBound);
    auto dataCost = new ceres::AutoDiffCostFunction<DataCost<Dim>, 1, Dim, Dim>(
        new DataCost<Dim>(obs, settings.commonSettings.residualLowerBound));
    problem.AddResidualBlock(dataCost,
        loss,
        getBlockPtr(X, obs.weights.lowerIndex),
        getBlockPtr(X, obs.weights.upperIndex()));
  }{
    int regOrder = settings.commonSettings.regOrder;
    int regCount = sampling.count() - regOrder;
    int paramBlockCount = regOrder + 1;
    auto regCost = makeCeresRegCost(Dim, new RegCost<Dim>(settings),
        paramBlockCount);
    auto loss = makeLossFunction(settings.regLoss,
              settings.commonSettings.residualLowerBound);
    for (int i = 0; i < regCount; i++) {
      std::vector<double*> blocks(paramBlockCount);
      for (int j = 0; j < paramBlockCount; j++) {
        int index = i + j;
        blocks[j] = getBlockPtr(X, index);
      }
      problem.AddResidualBlock(regCost,
          loss,
          blocks);
    }
  }
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = false;
  options.max_num_iterations = settings.commonSettings.iters;
  options.linear_solver_type = settings.solverType;
  options.num_threads = 6;

  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  return transpose(X);
}


};
}

#endif /* SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_ */
