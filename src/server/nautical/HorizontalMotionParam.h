/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  An abstract class that models a vector field to look-up for instance wind or
 *  current, based on some paramters.
 *
 *  It is intended to be used in an optimization context, and therefore
 *  uses the adouble type.
 */

#ifndef HORIZONTALMOTIONPARAM_H_
#define HORIZONTALMOTIONPARAM_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <adolc/adouble.h>

namespace sail {

class Nav;
class HorizontalMotionParam {
 public:
  // Returns the number of parameters used to
  // parameterize the vector field
  virtual int paramCount() = 0;

  // Initializes the parameters used to parameterize the vector field
  virtual void initialize(double *outParams) = 0;

  // Returns the local Horizontal motion at a Nav
  HorizontalMotion<adouble> get(const Nav &nav, adouble *params) = 0;
  virtual ~HorizontalMotionParam() {}
};

class ConstantHorizontalMotionParam : public HorizontalMotionParam {
 public:
  int paramCount() {return 2;}

  void initialize(double *outParams) {
    outParams[0] = 0;
    outParams[1] = 0;
  }

  HorizontalMotion<adouble> get(const Nav &nav, adouble *params) {
    return HorizontalMotion<adouble>(Velocity<adouble>::metersPerSecond(params[0]),
                                     Velocity<adouble>::metersPerSecond(params[1]));
  }
 private:
};

class ZeroHorizontalMotionParam : public HorizontalMotionParam {
 public:
  int paramCount() {return 0;}

  void initialize(double *outParams) {}

  HorizontalMotion<adouble> get(const Nav &nav, adouble *params) {
    return HorizontalMotion<adouble>(Velocity<adouble>::metersPerSecond(0),
                                     Velocity<adouble>::metersPerSecond(0));
  }
 private:
};

} /* namespace mmm */

#endif /* HORIZONTALMOTIONPARAM_H_ */
