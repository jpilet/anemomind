/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_MATH_NORMCONSTRAINED_H_
#define SERVER_MATH_NORMCONSTRAINED_H_

#include <server/common/LineKM.h>


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
  LineKM map(0, settings.iters, settings.initialWeight, settings.finalWeight);
  for (int i = 0; i < settings.iters; i++) {
    auto w = map(i);
    MatType Xhat = (1.0/X.norm())*X;
    MatType AtA = AtAbase + settings.finalWeight*Xhat*Xhat.transpose()
        + w*MatType::Identity(n, n);
    MatType AtB = AtAbase + settings.finalWeight*Xhat + w*Xhat;
    //Eigen::MatrixXd A;
    X = AtA.ldlt().solve(AtB);
  }
  return X;
}

}
}



#endif /* SERVER_MATH_NORMCONSTRAINED_H_ */
