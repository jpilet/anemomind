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

#ifndef BANDED_SOLVER
#define BANDED_SOLVER

#include <server/math/nonlinear/SignalUtils.h>

namespace sail {
namespace BandedSolver {

struct Settings {
 // How much regularization there should be.
 double lambda = 1.0;

 // The order of the regularization
 int regOrder = 1;

 // The number of iterations.
 int iters = 4;

 // For the banded matrix solver.
 double tol = 1.0e-6;

 // Lower threshold for residuals when majorizing them.
 double residualLowerBound = 0.0001;
};

MDArray2d initialize(int sampleCount, int dim);

template <int Dim, typename DataCost>
void accumulateData(DataCost dataCost, Array<Observation<Dim> > observations, MDArray2d X,
    BandMat<double> *AtA, MDArray2d *AtB, const Settings &settings) {
  assert(isFinite(X));
  for (const Observation<Dim> &obs: observations) {
    auto r = obs.calcResidual(X);
    assert(!std::isnan(r));
    auto q = majorizeCostFunction<DataCost>(dataCost, r, settings.residualLowerBound);
    assert(isFinite(q.a));
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
  for (int i = 0; i < n; i++) {
    double r = calcResidual<Dim>(difs, i);
    auto maj = majorizeCostFunction(regCost, r, settings.residualLowerBound);
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
    accumulateData<Dim, DataCost>(dataCost, observations, X, &AtA, &AtB, settings);
    accumulateReg<Dim, RegCost>(regCost, regCoefs, X, settings, &AtA);
    assert(isFinite(AtA.data()) && isFinite(AtB));
    if (bandMatGaussElimDestructive(&AtA, &AtB, settings.tol)) {
      return AtB;
    }
    return MDArray2d();
}

template <int Dim>
Array<Observation<Dim> > filterObservations(Sampling s,
    const Array<Observation<Dim> > &observations) {
    return observations.slice([=](const Observation<1> &obs) {
      return s.valid(obs.weights);
    });
}


/*
 ******* This is the main function to call in order to solve the problem.
 *
 * Dim: The dimension of each sample. For instance, if we are fitting a trajectory
 *   in the XY plane, it is 2.
 *
 * DataCost: A cost function to be applied to the data residuals, like one of those
 *   in SignalUtils.h, such as AbsCost.
 *
 * RegCost: A cost function applied to the regularization residuals.
 *
 *
 * This denoising algorithm is based on the total variation denoising
 * algorithm used in image processing: It tends to suppress noise, while preserving features
 * in the signal (such as sharp buoy turns) and that is what we want, right? But the
 * regularization term is second order, meaning that acceleration is penalized. Since I
 * regularize using l1 norm, the problem becomes nonsmooth and it becomes difficult to
 * evaluate derivatives accurately. The Majorize-Minimize algorithm has proven effective
 * for this nonsmooth denoising problem. In each iteration, it majorizes the objective
 * function with a quadratic, and then minimizes that quadratic (by solving a linear system).
 * The X that minimizes that quadratic is also guaranteed to at least not increase the value
 * of the objective function. Thus, no line search or damping is needed.
 *
 */
template <int Dim, typename DataCost, typename RegCost>
MDArray2d solve(
    DataCost dataCost, RegCost regCost,
    Sampling sampling, Array<Observation<Dim> > observations0, Settings settings,
    MDArray2d initialX = MDArray2d()) {

  auto observations = observations0.slice([](const Observation<Dim> &obs) {
    return obs.isFinite();
  });
  Arrayd regCoefs = makeRegCoefs(settings.regOrder);

  MDArray2d X = (initialX.empty()? initialize(sampling.count(), Dim) : initialX);
  for (int i = 0; i < settings.iters; i++) {
    auto nextX = iterate<Dim, DataCost, RegCost>(dataCost, regCost,
        sampling, observations, settings, X, regCoefs);
    if (nextX.empty()) {
      return X;
    } else {
      X = nextX;
    }
  }
  return X;
}

}
}



#endif
