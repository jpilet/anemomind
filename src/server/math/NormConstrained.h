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


namespace sail {
namespace NormConstrained {

struct Settings {
  double initialWeight = 0.1;
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
  auto finalWeight = sqr(settings.finalWeight);
  for (int i = 0; i < settings.iters; i++) {
    auto w = sqr(exp(map(i)));
    std::cout << EXPR_AND_VAL_AS_STRING(w) << std::endl;
    MatType Xhat = (1.0/X.norm())*X;
    MatType AtA = AtAbase + finalWeight*Xhat*Xhat.transpose()
        + w*MatType::Identity(n, n);
    MatType AtB = AtAbase + finalWeight*Xhat + w*Xhat;
    X = AtA.inverse()*AtB;
    std::cout << EXPR_AND_VAL_AS_STRING(X) << std::endl;
  }
  return X;
}

}
}



#endif /* SERVER_MATH_NORMCONSTRAINED_H_ */
