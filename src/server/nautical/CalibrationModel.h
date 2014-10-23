/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CALIBRATIONMODEL_H_
#define CALIBRATIONMODEL_H_

#include <server/nautical/SpeedCalib.h>

namespace sail {

/*
 * Use SI units whenever
 * the PhysicalQuantity type is not used.
 */

template <typename T>
class Corrector {
 public:
  virtual int paramCount() = 0;
  virtual void initialize(T *dst) = 0;
  virtual T correct(T *calibParameters, T x) const = 0;
  virtual ~Corrector() {}
};

template <typename T>
class OffsetCorrector : public Corrector<T> {
 public:
  int paramCount() {return 1;}
  void initialize(T *dst) {dst[0] = 0;}
  T correct(T *calibParameters, T x) const {
    return x + calibParameters[0];
  }
};

template <typename T>
class SpeedCorrector : public Corrector<T> {
 public:
  int paramCount() {return 4;}
  void initialize(T *dst) {
    dst[0] = SpeedCalib<T>::initKParam();
    dst[1] = SpeedCalib<T>::initMParam();
    dst[2] = SpeedCalib<T>::initCParam();
    dst[3] = SpeedCalib<T>::initAlphaParam();
  }
  T correct(T *calibParameters, T x) const {
    SpeedCalib<T> calib(calibParameters[0],
        calibParameters[1], calibParameters[2],
        calibParameters[3]);
    return calib.eval(Velocity<T>::metersPerSeconds(x)).metersPerSecond(x);
  }
 private:
};

/*
 * Given calibrated values of AWA and AWS, computes a
 * correction angle.
 */
template <typename T>
class DriftAngle {
 public:
  virtual int paramCount() {return 2;}
  virtual void initialize(T *dst) {
    dst[0] = 0;   // Maximum value of the
    dst[1] = -2;  // Slope
  }
  virtual T correct(T *params, T x) const {
    return params[0]*exp(-expline(params[1])*x);
  }

  virtual ~DriftAngle() {}
 private:
};

template <typename T>
class CorrectorSet {
 public:
  virtual Corrector<T> &compassAngleCorrector() = 0;
  virtual Corrector<T> &compassAngleCorrector() = 0;
 private:
};

}

#endif /* CALIBRATIONMODEL_H_ */
