/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_COMMON_SAMPLING_H_
#define SERVER_COMMON_SAMPLING_H_

#include <server/common/LineKM.h>
#include <server/common/MDArray.h>
#include <server/math/BandMat.h>
#include <server/common/math.h>
#include <server/math/Majorize.h>

namespace sail {

class Sampling {
 public:
  Sampling() : _sampleCount(0) {}
  Sampling(int count, double lower, double upper);

  struct Weights {
   int lowerIndex;
   double lowerWeight, upperWeight;

   int upperIndex() const {
     return lowerIndex + 1;
   }

   void accumulateAtA(double squaredWeight, BandMat<double> *dst) const {
     int inds[2] = {lowerIndex, upperIndex()};
     double weights[2] = {lowerWeight, upperWeight};
     (*dst).addNormalEq(squaredWeight, 2, inds, weights);
   }
  };

  Weights represent(double x) const;

  const LineKM &indexToX() const {
    return _indexToX;
  }

  const LineKM &xToIndex() const {
    return _xToIndex;
  }

  double period() const {
    return _indexToX.getK();
  }

  int count() const {
    return _sampleCount;
  }
 private:
  LineKM _indexToX, _xToIndex;
  int _sampleCount;
};

template <int N>
struct Observation {
  Sampling::Weights weights;
  double data[N];

  double calcResidual(const MDArray2d &X) const {
    double squaredDist = 0.0;
    for (int i = 0; i < N; i++) {
      squaredDist = weights.lowerWeight*X(weights.lowerIndex, i) +
                    weights.upperWeight*X(weights.upperIndex(), i);
    }
    return sqrt(squaredDist);
  }

  void accumulateNormalEqs(double squaredWeight,
      BandMat<double> *dstAtA, MDArray2d *dstAtB) const {
    weights.accumulateAtA(squaredWeight, dstAtA);
    for (int i = 0; i < N; i++) {
      (*dstAtB)(weights.lowerIndex, i) += squaredWeight*weights.lowerWeight*data[i];
      (*dstAtB)(weights.upperIndex(), i) += squaredWeight*weights.upperWeight*data[i];
    }
  }
};



/*
 * Cost functions to penalize the residuals.
 */
class RobustCost {
 public:
  RobustCost(double sigma) :
    _sigma2(sigma*sigma) {}

  double eval(double x) const {
    double x2 = sqr(x);
    return x2*_sigma2/(x2 + _sigma2) - _sigma2;
  }

  double evalDerivative(double x) const {
    double f = _sigma2/(_sigma2 + sqr(x));
    return 2*x*sqr(f);
  }
 private:
  double _sigma2;
};

////// To evaluate a scaled function: For a robust cost function, scaling makes the valley narrower,
////// and the inlier cost lower compared to the outlier cost. Like Julien's paper with the title
////// "Fast Non-Rigid Surface Detection, Registration and Realistic Augmentation".
template <typename CostFunction>
double evalScaled(const CostFunction &f, double x, double scale) {
  return scale*f.eval(scale*x);
}

template <typename CostFunction>
double evalDerivativeScaled(const CostFunction &f, double x, double scale) {
  return scale*scale*f.evalDerivative(scale*x);
}

////// This will majorize the cost function at x, so that
////// we find a quadratic function which is greater than the cost function
////// everywhere, whose derivative and value at x is that of f (but with the constant term omitted).
////// This lets us solve a quadratic minimization problem in every iteration.
template <typename CostFunction>
MajQuad majorizeCostFunction(const CostFunction &f, double x, double s = 1.0) {
  return MajQuad::majorize(x, evalScaled(f, x, s), evalDerivativeScaled(f, x, s), 0.0);
}

} /* namespace sail */

#endif /* SERVER_COMMON_SAMPLING_H_ */
