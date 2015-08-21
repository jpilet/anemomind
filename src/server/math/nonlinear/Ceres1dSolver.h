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
T softSqrt(T x, T lb) {
  if (x <= T(0)) {
    return T(0);
  } else if (x < lb) {
    T sqrtLb = sqrt(lb);
    auto dif = lb - x;
    return sqrtLb - 0.5*dif/sqrtLb;
  }
  return sqrt(x);
}


/*
 * I am not sure how to construct
 * an MDArray from a constant pointer,
 * so let's copy it.
 */
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


/*
 * This is the data fitness cost.
 */
template <int Dim>
class DataCost {
 public:
  DataCost(const Sampling &sampling,
      Observation<Dim> observation,
      Settings settings, int index) :
        _sampling(sampling),
        _observation(observation),
        _settings(settings), _index(index) {}

  template<typename T>
      bool operator()(const T* x, const T* y, T* residual) const {
    T r2(0.0);
    auto w = _observation.weights;
    for (int i = 0; i < Dim; i++) {
      r2 += sqr(w.lowerWeight*x[i] + w.upperWeight*y[i] - _observation.data[i]);
    }
    auto dataResidual = softSqrt<T>(r2, T(_settings.commonSettings.residualLowerBound));
    //std::cout << EXPR_AND_VAL_AS_STRING(dataResidual) << std::endl;
    if (_index < 30) {
      std::cout << " dataResidual (" << _index << ") = " << dataResidual << "\n";
    }
    residual[0] = dataResidual;
    return true;
  }
 private:
  Sampling _sampling;
  Observation<Dim> _observation;
  Settings _settings;
  int _index;
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

    T r2(0);
    for (int j = 0; j < Dim; j++) {
      r2 += sqr(difs(0, j));
    }
    auto regResidual = softSqrt(_settings.commonSettings.lambda*r2,
        T(_settings.commonSettings.residualLowerBound));
    residual[0] = regResidual;
    return true;
  }

  int outDims() const {
    return 1;
  }

  int inDims() const {
    return Dim;
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
  X.setAll(1.0);
  CHECK(X.isContinuous());

  ceres::Problem problem;
  for (int i = 0; i < sampling.count(); i++) {
    problem.AddParameterBlock(X.sliceCol(i).ptr(), Dim);
  }

  int i = 0;
  for (Observation<Dim> obs: observations) {
    i++;
    auto dataCost = new ceres::AutoDiffCostFunction<DataCost<Dim>, 1, Dim, Dim>(
        new DataCost<Dim>(sampling, obs, settings, i));
    problem.AddResidualBlock(dataCost,
        makeLossFunction(settings.dataLoss,
        settings.commonSettings.residualLowerBound),
        getBlockPtr(X, obs.weights.lowerIndex),
        getBlockPtr(X, obs.weights.upperIndex()));
  }/*{
    int regOrder = settings.commonSettings.regOrder;
    int regCount = sampling.count() - regOrder;
    int paramBlockCount = regOrder + 1;
    for (int i = 0; i < regCount; i++) {
      std::vector<double*> blocks(paramBlockCount);
      for (int j = 0; j < paramBlockCount; j++) {
        int index = i + j;
        blocks[j] = getBlockPtr(X, index);
      }
      auto regCost = makeCeresRegCost(Dim, new RegCost<Dim>(settings),
          paramBlockCount);
      problem.AddResidualBlock(regCost,
          makeLossFunction(settings.regLoss,
          settings.commonSettings.residualLowerBound),
          blocks);
    }
  }*/
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  //options.max_num_iterations = settings.commonSettings.iters;
  //options.linear_solver_type = settings.solverType;

  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;
  return transpose(X);
}


};
}

#endif /* SERVER_MATH_NONLINEAR_CERES1DSOLVER_H_ */
