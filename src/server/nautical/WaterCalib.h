/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef WATERCALIB_H_
#define WATERCALIB_H_

#include <server/common/Array.h>
#include <server/nautical/Nav.h>
#include <server/nautical/SpeedCalib.h>
#include <server/nautical/HorizontalMotionParam.h>

namespace sail {


class WaterCalib {
 public:
  WaterCalib(const HorizontalMotionParam &param,
    Velocity<double> sigma, Velocity<double> initR);


  int paramCount() const {
    return 5 + _param.paramCount();
  }

  Arrayd optimize(Array<Nav> navs) const;

  template <typename T>
  SpeedCalib<T> makeSpeedCalib(T *X) const {
    return SpeedCalib<T>(wcK(X), wcM(X), wcC(X), wcAlpha(X));
  }

  template <typename T> T unwrap(Velocity<T> x) const {return x.knots();}
 private:
  Arrayd makeInitialParams() const;
  void initialize(double *outParams) const;
  Velocity<double> _sigma, _initR;
  const HorizontalMotionParam &_param;
  template <typename T> T &wcK(T *x) const {return x[0];}
  template <typename T> T &wcM(T *x) const {return x[1];}
  template <typename T> T &wcC(T *x) const {return x[2];}
  template <typename T> T &wcAlpha(T *x) const {return x[3];}
  template <typename T> T &magOffset(T *x) const {return x[4];}

  //SpeedCalib(T k, T m, T c, T alpha) :
};

}

#endif /* WATERCALIB_H_ */
