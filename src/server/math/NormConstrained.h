/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_NORMCONSTRAINED_H_
#define SERVER_MATH_NORMCONSTRAINED_H_

#include <server/common/LineKM.h>
#include <server/common/string.h>
#include <iostream>
#include <Eigen/Dense>
#include <server/common/math.h>
#include <server/common/logging.h>


namespace sail {
namespace NormConstrained {

struct Settings {
  double initialWeight = 0.01;
  double finalWeight = 10000.0;
  int iters = 30;
};

/*
 * Minimize |A*X - B|^2 subject to |X| = 1
 */
template <typename MatType>
MatType minimizeNormConstrained(MatType A, MatType B,
    MatType initX, const Settings &settings) {
  int n = A.cols();
  MatType X = initX;
  MatType AtAbase = A.transpose()*A;
  MatType AtBbase = A.transpose()*B;
  LineKM map(0, settings.iters-1, log(settings.initialWeight), log(settings.finalWeight));
  auto finalWeight = settings.finalWeight;
  for (int i = 0; i < settings.iters; i++) {
    auto w = exp(map(i));
    typedef decltype(X.norm()) T;
    MatType Xhat = T(T(1.0)/X.norm())*X;
    MatType AtAvar = w*MatType::Identity(n, n);
    MatType AtBvar = w*Xhat;

    MatType AtA = AtAbase + AtAvar;
    MatType AtB = AtBbase + AtBvar;
    X = AtA.lu().solve(AtB);
  }
  return X;
}

}
}



#endif /* SERVER_MATH_NORMCONSTRAINED_H_ */
