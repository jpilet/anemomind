/*
 *  Created on: 2014-04-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Implementation of Geman-McClure robust estimator
 *
 *
 *
 *
 *  === Derivation ===
 *  Let the robust cost function be
 *    f(x, w) = w²*x² + (1 - w)²*sigma²
 *
 *  w is an unknown weight that controls the inlierness of an observation x
 *  and sigma is an inlier threshold.
 *
 *  Minimize this cost function w.r.t. w:
 *  df/dw = 0 <=> w = sigma²/(sigma² + x²)
 *
 *  Insertion of optimal w in f(x, w) yields, after simplifications,
 *
 *  f(x) = sigma²*x²/(sigma² + x²)
 */

#ifndef GEMANMCCLURE_H_
#define GEMANMCCLURE_H_

#include <server/common/math.h>
#include <server/common/Array.h>
#include <server/common/Function.h>
#include <memory>

namespace sail {

class GemanMcClure {
 public:
  GemanMcClure();

  /*
   * sigma: An inlier threshold. For instance, in vision problems it could be e.g. 10 pixels
   * initR: Controls the trade-off between rate of convergence and robustness to local optima. A higher value
   *        means slower convergence but better robustness to local optima.
   * count: The number of observations.
   * dim:   The dimension of every observation. In the 2d image plane for instance, dim = 2.
   */
  GemanMcClure(double sigma, double initR, int count, int dim);

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

  double getResidualWeight(int index) const {
    return getWeight(index/_dim);
  }



  double calcOutlierCost() const;

  /*
   *  This number can be appended to the residual vector
   *  in a least-squares problem so that the cost function
   *  behaves the way we would expect.
   */
  double calcOutlierResidual() const {
    return sqrt(calcOutlierCost());
  }

  bool isInlier(int index, Arrayd residuals) const {
    return getSquaredError(index, residuals) <= _sigma2;
  }

  bool isOutlier(int index, Arrayd residuals) const {
    return !isInlier(index, residuals);
  }
 private:
  int _iters, _dim;
  double _sigma2;
  Arrayd _R2sum;

  double getSquaredError(int index, Arrayd residuals) const;

  /*
   * For least-squares problems,
   * provides a weight for each observation.
   *
   * See getResidualWeight for the weight multiply with the residuals.
   *
   * This method doesn't strictly have to be private,
   * but I let it be private now, because most likely one wants
   * to use getResidualWeight.
   */
  double getWeight(int index) const {
    return calcWeightFromSquared(avgSquareResidual(index));
  }
};

/*
 * This function can wrap a non-robust least-square cost function
 * in nonlinear least squares problems.
 */
class GemanMcClureFunction : public Function {
 public:
  GemanMcClureFunction(double sigma, double initR, int dim,
        std::shared_ptr<Function> fun);
  int inDims() {return _fun->inDims();}
  int outDims() {return _fun->outDims() + 1;} // One extra for the outlier residual.
  void eval(double *Xin, double *Fout, double *Jout);
  const GemanMcClure &gmc() const {return _gmc;}
 private:
  bool _initialized;
  GemanMcClure _gmc;
  std::shared_ptr<Function> _fun;
  void evalNoJacobian(double *Xin, double *Fout);
  void evalWithJacobian(double *Xin, double *Fout, double *Jout);
};

} /* namespace sail */

#endif /* GEMANMCCLURE_H_ */
