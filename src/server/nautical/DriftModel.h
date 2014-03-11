/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DRIFTMODEL_H_
#define DRIFTMODEL_H_

#include <server/math/armaadolc.h>
#include <adolc/adouble.h>
#include <server/nautical/Nav.h>

namespace sail {

class BoatData;


/*
 * This is how it works:
 * The little propeller on the keel measures the speed of water
 * along the hull of the boat. This speed is assumed to be the
 * actual speed of the boat w.r.t. the water. However, due to drift,
 * the direction at which the boat moves w.r.t. water may not
 * be the same as the direction at which the keel is pointing.
 *
 * This object provides a correction angle.
 *
 */
class DriftModel {
 public:
  virtual int paramCount() = 0;
  virtual void initializeParameters(double *x) = 0;

  virtual adouble calcCorrectionAngle(BoatData *data, const Nav &nav, adouble *Xin) = 0;

  virtual ~DriftModel() {}
};

class SimplestDriftModel : public DriftModel {
 public:
  int paramCount() {return 0;}
  void initializeParameters(double *x) {}
  adouble calcCorrectionAngle(BoatData *data, const Nav &nav, adouble *Xin) {return 0;}
};

// Assume there is a small error in the course when we sail close to the wind.
class SinusDriftAngle : public DriftModel {
 public:
  int paramCount() {return 0;}
  void initializeParameters(double *x) {}
  adouble calcCorrectionAngle(BoatData *data, const Nav &nav, adouble *Xin);
 private:
  adouble preliminaryCourseErrorDueToDrift(adouble awaRadians);
};

} /* namespace mmm */

#endif /* DRIFTMODEL_H_ */
