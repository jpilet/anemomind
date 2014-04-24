/*
 *  Created on: 2014-04-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Implementation of Geman-McClure robust estimator
 */

#ifndef GEMANMCCLURE_H_
#define GEMANMCCLURE_H_

#include <server/common/math.h>

namespace sail {

class GemanMCClure {
 public:
  GemanMCClure();

  /*
   * sigma: An inlier threshold. For instance, in vision problems it could be e.g. 10 pixels
   * initR: Controls the trade-off between rate of convergence and robustness to local optima. A higher value
   *        means slower convergence but better robustness to local optima.
   * count: The number of observations.
   * dim:   The dimension of every observation. In the 2d image plane for instance, dim = 2.
   */
  GemanMCClure(double sigma, double initR, int count, int dim);

  /*
   * Call this function with the residuals for
   * every Jacobian evaluation in a non-linear lsq
   * algorithm
   */
  void addResiduals(Arrayd residuals);

  double avgSquareResidual(int index) const {
    return _R2sum[index]/_iters;
  }

  double avgResidual(int index) const {
    return sqrt(avgSquareResidual(index));
  }

  double calcWeightFromSquared(double r2) const {
    return _sigma2/(_sigma2 + r2);
  }

  double calcCostFromSquared(double r2) const {
    return _sigma2*r2/(_sigma2 + r2);
  }

  double calcWeight(double residual) const {
    return calcWeightFromSquared(sqr(residual));
  }

  double calcCost(double residual) const {
    return calcCostFromSquared(sqr(residual));
  }

  /*
   * For least-squares problems, multiply the residuals
   * and rows of the Jacobian with these weights.
   */
  double getWeight(int index) const {
    return calcWeightFromSquared(avgSquareResidual(index));
  }

  double calcOutlierCost(Arrayd residuals) const;
  double calcOutlierResidual(Arrayd residuals) const;
 private:
  int _iters, _dim;
  double _sigma2;
  Arrayd _R2sum;
};

} /* namespace sail */

#endif /* GEMANMCCLURE_H_ */
