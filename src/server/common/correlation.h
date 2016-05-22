/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CORRELATION_H_
#define CORRELATION_H_

#include <server/common/MeanAndVar.h>

namespace sail {

template <typename T>
double normalizedCrossCorrelation(Array<T> X, Array<T> Y, T unit = T(1)) {
  int count = X.size();
  assert(count == Y.size());
  MeanAndVar xstat, ystat;
  for (int i = 0; i < count; i++) {
    xstat.add(X[i]/unit);
    ystat.add(Y[i]/unit);
  }
  double xmean = xstat.mean();
  double ymean = ystat.mean();
  double corr = 0.0;
  for (int i = 0; i < count; i++) {
    corr += (double(X[i]/unit) - xmean)*(double(Y[i]/unit) - ymean);
  }
  return corr/(count*xstat.standardDeviation()*ystat.standardDeviation());
}

}

#endif /* CORRELATION_H_ */
