/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_SPARSECURVEFIT_H_
#define SERVER_MATH_NONLINEAR_SPARSECURVEFIT_H_

#include <server/math/SparsityConstrained.h>
#include <server/common/logging.h>
#include <server/math/nonlinear/SignalUtils.h>
#include <server/common/MDArray.h>
#include <Eigen/Sparse>
#include <vector>

namespace sail {
namespace SparseCurveFit {

typedef Eigen::Triplet<double> Triplet;

Array<Spani> makeReg(int order, int firstRowOffset, int firstColOffset,
    int dim, int count, std::vector<Triplet> *dst);

template <int Dim>
Array<Spani> makeDataFitness(int firstRowOffset, int firstColOffset,
    Array<Observation<Dim> > observations, int sampleCount,
    std::vector<Triplet> *dst, Eigen::VectorXd *rhs) {
  int count = observations.size();
  int totalDim = Dim*count;
  int sampleDim = Dim*sampleCount;

  for (int i = 0; i < count; i++) {
    Observation<Dim> x = observations[i];
    int colA = firstColOffset + Dim*x.weights.lowerIndex;
    int colB = firstColOffset + Dim*x.weights.upperIndex();
    int iDim = i*Dim;
    int rowOffset = firstRowOffset + iDim;
    for (int k = 0; k < Dim; i++) {
      int row = rowOffset + k;

      // Express a point on the curve as a linear combination of the samples.
      dst->push_back(Triplet(row, colA + k, x.lowerWeight));
      dst->push_back(Triplet(row, colB + k, x.upperWeight));

      (*rhs)(row) = x.data[k];
      int slackCol = firstColOffset + sampleDim + iDim + k;

      // On the same row as the observation,
      // there is also a slack variable. For inliers,
      // this slack variable will be zero. For outliers,
      // it can be anything that minimizes the datafit.
      // The algorithm will automatically select the best
      // choice of slack variables to be put to zero.
      dst->push_back(Triplet(row, slackCol, 1.0));

      // This row is part of the constraint, that
      // forces some slack variables to zero.
      dst->push_back(Triplet(row + totalDim, slackCol, 1.0));
    }
  }
  return Spani(0, count).map<Spani>([&](int index) {
    int offset = firstRowOffset + totalDim + Dim*index;
    return Spani(offset, offset + Dim);
  });
}



struct Settings {
  // These are the settings of the underlying algorithm used to solve the problem.
  SparsityConstrained::Settings settings;

  // Select this fraction of all observations to use as inliers, ignore the others.
  // An alternative to this would have been to have some inlier threshold, sigma, so that
  // if the distance between the observation and the fitted curve is less than sigma, then
  // the observation should be treated as an inlier. In that case, we should know more or less
  // the noise distribution, but we might not know it. The problem with choosing sigma is that
  // a value that is too low will make us overfit to a few samples, whereas a value which is too high
  // will also include outliers among the samples treated as inliers.
  // On the other hand, when picking
  // a fraction of all the measurements and use to perform the fit, we should be sure that the inlier
  // rate is not lower than that fraction, because then it is going to break. And we should not choose
  // the fraction too low, because then we are going to overfit to a few samples. In either case,
  // no matter if we choose fraction-based outlier rejection or threshold-based rejection, we will
  // have to tune a parameter because one size doesn't fit all. But in most cases, I believe that it
  // is fair to say that at least around the best 60% of the samples should be high quality
  // observations even for crappy gps devices.
  double inlierRate = 0.6;

  // How many discontinuities in some derivative (of order regOrder) we expect. The more discontinuities
  // we allow for, the more complex the fitted curve can be. These discontinuities are what makes
  // the curve "sparse", because the discontinuities are sparse. At most places, there is no discontinuity.
  // The discontinuity count can be tuned so that there is maybe one discontinuity every six seconds or
  // so in a temporal signal. Discontinuities are used to model features in the underlying noise-free signal.
  // I hope that this way of regularizing will be very robust to different amounts of noise in the
  // measurements. And the algorithm will automatically select the locations for the discontinuities where
  // they are most needed, e.g. at buoy turns.
  double discontinuityCount = -1;

  // For which derivative order we count the discontinuities.
  // A regOrder of 1 would mean a constant piecewise curve.
  // A regOrder of 2 means a continuous curve of piecewise straight line segments,
  //   or its first derivative being piecewise constant.
  // A regorder of 3 means that the second derivative of the curve is piecewise constant.
  int regOrder = 3;
};



/*
 *
 * The main function to solve the problem.
 *
 */
template <int Dim>
MDArray2d fit(const Settings &settings, int sampleCount, Array<Observation<Dim> > observations) {
  CHECK(0 <= settings.discontinuityCount);
}


}
}

#endif /* SERVER_MATH_NONLINEAR_SPARSECURVEFIT_H_ */
