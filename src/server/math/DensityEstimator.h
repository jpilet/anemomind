/*
 *  Created on: 2014-09-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DENSITYESTIMATOR_H_
#define DENSITYESTIMATOR_H_

#include <server/common/Array.h>

namespace sail {

/*
 * A common interface for density estimators.
 * This lets us easily replace the density estimation algorithm
 * with a faster one, once we have implemented that.
 */
class DensityEstimator {
 public:
  /*
   * It might be a good idea not to require
   * this method to be const. Maybe in a
   * sophisticated implementation, we might
   * want to optimize the lookup procedure
   * based on the query.
   */
  virtual double density(const Arrayd &point) = 0;

  virtual ~DensityEstimator() {}
};

/*
 * For a start, provide the simplest possible density estimator.
 * Estimating the density in a point is O(n), n being the number of
 * training points. It is not scalable, but better than nothing.
 */
class NaiveDensityEstimator : public DensityEstimator {
 public:
  NaiveDensityEstimator(double bandwidth,
        Array<Arrayd> samples);
  double density(const Arrayd &point);

  int dims() const {
    return _samples[0].size();
  }
 private:
  double _bandwidth;
  Array<Arrayd> _samples;
  static bool consistentDims(Array<Arrayd> samples);
};

}

#endif /* DENSITYESTIMATOR_H_ */
