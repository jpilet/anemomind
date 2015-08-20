/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  General implementation of an iterative least squares approach
 *  to solve one dimensional fitting problems with a data term
 *  and a regularization term. Various cost functions can be
 *  applied to the data and the regularization residuals. The order
 *  of the regularization can also be controlled, to penalize for
 *  instance the first or the second derivative. In every iteration,
 *  the objective function is majorized by a quadratic and a least squares
 *  problem is solved, which amounts to solving a banded linear system
 *  of equations from the normal equations.
 *
 *  TODO: Implement total variation and all other denoising
 *  using this solver.
 *
 */

#include <server/math/nonlinear/SignalUtils.h>

namespace sail {
namespace BandedSolver {

struct Settings {
 // How much regularization there should be.
 double lambda = 1.0;

 // The order of the regularization
 int regOrder = 1;

 // The number of iterations.
 int iters = 30;

 // For the banded matrix solver.
 double tol = 1.0e-6;
};


MDArray2d initialize(int sampleCount, int dim) {
  MDArray2d x(sampleCount, dim);
  x.setAll(0.0);
  return x;
}

template <int Dim, typename DataCost>
void accumulateData(DataCost dataCost, Array<Observation<Dim> > observations, MDArray2d X,
    BandMat<double> *AtA, MDArray2d *AtB) {
    for (const Observation<Dim> &obs: observations) {
      auto r = obs.calcResidual(X);
      if (std::isnan(r)) {
        std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;
        std::cout << EXPR_AND_VAL_AS_STRING(obs.weights.lowerIndex) << std::endl;
        std::cout << EXPR_AND_VAL_AS_STRING(obs.weights.upperIndex()) << std::endl;
        std::cout << EXPR_AND_VAL_AS_STRING(obs.data[0]) << std::endl;
      }
      assert(!std::isnan(r));
      auto q = majorizeCostFunction<DataCost>(dataCost, r);
      obs.accumulateNormalEqs(q.a, AtA, AtB);
    }
}

template <int Dim>
MDArray2d calcDifsInPlace(int regOrder, MDArray2d X) {
  assert(Dim == X.cols());
  if (regOrder == 0) {
    return X;
  } else {
    int rows = X.rows();
    int dstRows = rows-1;
    for (int i = 0; i < dstRows; i++) {
      for (int j = 0; j < Dim; j++) {
        X(i, j) = X(i, j) - X(i+1, j);
      }
    }
    return calcDifsInPlace<Dim>(regOrder-1, X.sliceRowsTo(dstRows));
  }
}

template <int Dim>
double calcResidual(const MDArray2d &X, int row) {
  double r2 = 0.0;
  for (int i = 0; i < Dim; i++) {
    r2 += sqr(X(row, i));
    assert(!std::isnan(r2));
  }
  return sqrt(r2);
}

template <int Dim, typename RegCost>
void accumulateReg(RegCost regCost, Arrayd regCoefs,
    MDArray2d X, Settings settings, BandMat<double> *AtA) {
  auto difs = calcDifsInPlace<Dim>(settings.regOrder, X.dup());
  int n = difs.rows();
  std::cout << EXPR_AND_VAL_AS_STRING(n) << std::endl;
  for (int i = 0; i < n; i++) {
    double r = calcResidual<Dim>(difs, i);
    auto maj = majorizeCostFunction(regCost, r);
    std::cout << EXPR_AND_VAL_AS_STRING(maj.a) << std::endl;
    AtA->addRegAt(i, regCoefs, settings.lambda*maj.a);
  }
}



template <int Dim, typename DataCost, typename RegCost>
MDArray2d iterate(DataCost dataCost, RegCost regCost,
      Sampling sampling, Array<Observation<Dim> > observations,
    Settings settings, MDArray2d X, Arrayd regCoefs) {
    assert(X.rows() == sampling.count());
    assert(X.cols() == Dim);
    BandMat<double> AtA(sampling.count(), sampling.count(),
        settings.regOrder, settings.regOrder);
    AtA.setAll(0.0);
    MDArray2d AtB(sampling.count(), Dim);
    AtB.setAll(0.0);
    accumulateData<Dim, DataCost>(dataCost, observations, X, &AtA, &AtB);
    accumulateReg<Dim, RegCost>(regCost, regCoefs, X, settings, &AtA);
    std::cout << EXPR_AND_VAL_AS_STRING(AtA.getDataForDebug()) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(AtB) << std::endl;
    if (bandMatGaussElimDestructive(&AtA, &AtB, settings.tol)) {
      return AtB;
    }
    return MDArray2d();
}

/*
 * Dim: The dimension of each sample. For instance, if we are fitting a trajectory
 *   in the XY plane, it is 2.
 *
 * DataCost: A cost function to be applied to the data residuals, like one of those
 *   in SignalUtils.h, such as AbsCost.
 *
 * RegCost: A cost function applied to the regularization residuals.
 *
 */
template <int Dim, typename DataCost, typename RegCost>
MDArray2d solve(
    DataCost dataCost, RegCost regCost,
    Sampling sampling, Array<Observation<Dim> > observations, Settings settings,
    MDArray2d initialX = MDArray2d()) {
  Arrayd regCoefs = BandMatInternal::makeCoefs(settings.regOrder);
  MDArray2d X = (initialX.empty()? initialize(sampling.count(), Dim) : initialX);
  for (int i = 0; i < settings.iters; i++) {
    std::cout << EXPR_AND_VAL_AS_STRING(i) << std::endl;
    auto nextX = iterate<Dim, DataCost, RegCost>(dataCost, regCost,
        sampling, observations, settings, X, regCoefs);
    if (nextX.empty()) {
      std::cout << "Stop!" << std::endl;
      return X;
    } else {
      X = nextX;
    }
  }
  return X;
}

}
}
