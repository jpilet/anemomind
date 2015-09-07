/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_NONLINEAR_SPARSEFILTER_H_
#define SERVER_MATH_NONLINEAR_SPARSEFILTER_H_

#include <server/math/SparsityConstrained.h>

namespace sail {
namespace SparseFilter {

struct Settings {
  SparsityConstrained::Settings spcstSettings;
  int regOrder;
};


template <int N>
MDArray2d filter(Sampling sampling, Array<Observation<N> > observations,
  int inlierCount, int discontinuityCount, Settings settings) {
  typedef Eigen::Triplet<double> Triplet;
  int signalDim = N*sampling.count();
  int dataDim = N*observations.size();
  int cols = signalDim + dataDim;
  int regCount = sampling.count() - settings.regOrder;
  int regDim = N*regCount;
  int rows = dataDim + dataDim + regDim;

  std::vector<Triplet> triplets;
  Eigen::VectorXd B = Eigen::VectorXd::Zero(rows);

  // Build the data part
  for (int i = 0; i < observations.size(); i++) {
    const Observation<N> &obs = observations[i];
    int row = N*i;
    int li = N*obs.weights.lowerIndex;
    int ui = N*obs.weights.upperIndex;
    for (int j = 0; j < N; j++) {
      triplets.push_back(Triplet(row + j, li + j, obs.weights.lowerWeight));
      triplets.push_back(Triplet(row + j, ui + j, obs.weights.upperWeight));
      B(row + j) = obs.data[j];
    }
  }

  // Build the slack part (observations that are classified as outliers have non-zero slack.)
  for (int i = 0; i < dataDim; i++) {
    int col = signalDim + i;
    triplets.push_back(Triplet(i, col, 1.0));
    triplets.push_back(Triplet(dataDim + i, col, 1.0));
  }

  Array<Spani> slackSpans(observations.size());
  for (int i = 0; i < observations.size(); i++) {
    int offset = dataDim + N*i;
    slackSpans[i] = Spani(offset, offset + N);
  }

  // Build the regularization part
  int regRowOffset = 2*dataDim;
  Arrayd coefs = makeRegCoefs(settings.regOrder);
  Array<Spani> regSpans(regCount);
  for (int i = 0; i < regCount; i++) {
    int rowOffset = regRowOffset + N*i;
    regSpans[i] = Spani(rowOffset, rowOffset + N);
    for (int k = 0; k < coefs.size(); k++) {
      double coef = coefs[k];
      int colOffset = N*(i + k);
      for (int j = 0; j < N; j++) {
        triplets.push_back(Triplet(rowOffset + j, colOffset + j, coefs));
      }
    }
  }

  Eigen::SparseMatrix<double> A(rows, cols);
  A.setFromTriplets(triplets.begin(), triplets.end());

  typedef SparsityConstrained::ConstraintGroup CstGroup;

  auto slackCst = CstGroup{slackSpans, inlierCount};
  auto regCst = CstGroup{regSpans, regCount - discontinuityCount};

  auto result = SparsityConstrained::solve(A, B,
    Array<CstGroup>{slackCst, regCst}, settings.spcstSettings);
}

}
}

#endif /* SERVER_MATH_NONLINEAR_SPARSEFILTER_H_ */
