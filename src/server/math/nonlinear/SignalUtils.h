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

#include <iostream>
#include <server/common/string.h>

namespace sail {

/*
 * Defines the sampling of a signal in time.
 * A sampling consists of the number of samples
 * accessed with the method count(), and the
 * function (indexToX()) that maps a sample index to the domain
 * where the signal is sampled.
 * Suppose for instance that we have a function
 * f: R -> R. We sample this signal at
 * locations x_0 = 1, x_1 = 3, x_2 = 5 and so on.
 * Then we have the function indexToX(i) = 2*i + 1.
 */
class Sampling {
 public:
  Sampling() : _sampleCount(0) {}
  Sampling(int count, double lower, double upper);
  Sampling(int count, LineKM indexToX);

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

   template <typename T>
   T eval(const Array<T> &x) const {
     return lowerWeight*x[lowerIndex] + upperWeight*x[upperIndex()];
   }

   template <typename T>
   void eval(const MDArray<T, 2> &X, T *dst) const {
     int cols = X.cols();
     for (int i = 0; i < cols; i++) {
       dst[i] = lowerWeight*X(lowerIndex, i) + upperWeight*X(upperIndex(), i);
     }
   }

   template <typename T>
   void evalDerivative(const MDArray<T, 2> &X, T *dst) const {
     int cols = X.cols();
     for (int i = 0; i < cols; i++) {
       dst[i] = X(upperIndex(), i) - X(lowerIndex, i);
     }
   }

   bool isFinite() const {
     return std::isfinite(lowerWeight) && std::isfinite(upperWeight);
   }
  };

  bool valid(const Weights &w) const {
    return w.upperIndex() < _sampleCount;
  }

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

  int lastIndex() const {
    return _sampleCount-1;
  }

  Arrayd makeX() const;

  bool operator==(const Sampling &other) const {
    return _indexToX == other.indexToX() &&
        _sampleCount == other._sampleCount;
  }
 private:
  LineKM _indexToX, _xToIndex;
  int _sampleCount;
};





/*
 * Encodes an observation of
 * a vector valued function
 * f: R -> R^N
 * If we are attempting to recover
 * an unknown function f from some data,
 * we might observe that for observation with index i,
 * x_i maps to y_i, where x_i belongs to R and
 * y_i belongs to R^N. x_i is encoded as a linear combination
 * of samples from the underlying signal.
 */
template <int N>
struct Observation {
  Sampling::Weights weights;
  double data[N];

  bool isFinite() const {
    if (weights.isFinite()) {
      for (int i = 0; i < N; i++) {
        if (!std::isfinite(data[i])) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  double calcResidual(const MDArray2d &X) const {
    double squaredDist = 0.0;
    for (int i = 0; i < N; i++) {
      squaredDist += sqr(weights.lowerWeight*X(weights.lowerIndex, i) +
                        weights.upperWeight*X(weights.upperIndex(), i) - data[i]);
    }
    return sqrt(squaredDist);
  }

  void accumulateNormalEqs(double squaredWeight,
      BandMat<double> *dstAtA, MDArray2d *dstAtB) const {
    assert(std::isfinite(squaredWeight));
    weights.accumulateAtA(squaredWeight, dstAtA);
    for (int i = 0; i < N; i++) {
      assert(std::isfinite(data[i]));
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

class AbsCost {
 public:
  double eval(double x) const {
    if (x < 0) {
      return eval(-x);
    }
    return x;
  }

  double evalDerivative(double x) const {
    // The lower bound parameter in for instance majorizeCostFunction ensures this does not happen.
    assert(x != 0);

    return (x < 0? -1 : 1);
  }
 private:
  double _tao;
};

class SquareCost {
 public:
  double eval(double x) const {
    return x*x;
  };

  double evalDerivative(double x) const {
    return 2.0*x;
  }
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
MajQuad majorizeCostFunction(const CostFunction &f, double x0, double lb = 0.0001,
    double s = 1.0) {
  double x = thresholdCloseTo0(x0, lb);
  return MajQuad::majorize(x, evalScaled(f, x, s), evalDerivativeScaled(f, x, s), 0.0);
}

} /* namespace sail */

#endif /* SERVER_COMMON_SAMPLING_H_ */
