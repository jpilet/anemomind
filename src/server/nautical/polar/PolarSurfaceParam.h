/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARSURFACEPARAM_H_
#define POLARSURFACEPARAM_H_

#include <server/nautical/polar/PolarCurveParam.h>

namespace sail {

class PolarSurfaceParam {
 public:
  PolarSurfaceParam(PolarCurveParam pcp,
      Velocity<double> maxTws, int twsLevelCount);
  PolarSurfaceParam();
 private:
  PolarCurveParam _polarCurveParam;
  Velocity<double> _maxTws;
  int _twsLevelCount;
};

}

#endif /* POLARSURFACEPARAM_H_ */
