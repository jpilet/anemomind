/*
 *  Created on: 2014-07-02
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ROBUSTSIGNALFIT_H_
#define ROBUSTSIGNALFIT_H_

#include <server/nautical/Grid.h>

namespace sail {

class RobustSignalFit {
 public:
  RobustSignalFit(LineStrip strip, Arrayd X, Arrayd Y, double sigma,
      double initR, int iters);
 private:
  LineStrip _strip;
  Arrayd _X, _Y, _fit;
  Arrayb _inliers;
};

}

#endif /* ROBUSTSIGNALFIT_H_ */
