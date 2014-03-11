/*
 *  Created on: 11 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef DRIFTMODEL_H_
#define DRIFTMODEL_H_

#include <server/math/armaadolc.h>
#include <adolc/adouble.h>
#include <server/nautical/Nav.h>

namespace sail {

class BoatData;



class DriftModel {
 public:
  virtual int paramCount() = 0;
  virtual void initializeParameters(double *x) = 0;

  virtual adouble calcCourseError(BoatData *data, const Nav &nav, adouble *Xin) = 0;

  virtual ~DriftModel() {}
};

class SimplestDriftModel : public DriftModel {
 public:
  int paramCount() {return 0;}
  void initializeParameters(double *x) {}
  adouble calcCourseError(BoatData *data, const Nav &nav, adouble *Xin) {return 0;}
};

// Assume there is a small error in the course when we sail close to the wind.
class SinusDriftAngle : public DriftModel {
 public:
  int paramCount() {return 0;}
  void initializeParameters(double *x) {}
  adouble calcCourseError(BoatData *data, const Nav &nav, adouble *Xin);
 private:
  adouble preliminaryCourseErrorDueToDrift(adouble awaRadians);
};

} /* namespace mmm */

#endif /* DRIFTMODEL_H_ */
