/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SynthBoatTrajectory.h"
#include <cmath>
#include <server/common/string.h>
#include <iostream>

namespace sail {


void SynthBoatTrajectory::WayPt::solveCircleTangentLine(
      const WayPt &a, bool posa,
      const WayPt &b, bool posb,
      Angle<double> *outAngleA, Angle<double> *outAngleB) {
  Vectorize<Length<double>, 2> dif = a.pos - b.pos;
  double C = sqrt(sqr(dif[0].meters()) + sqr(dif[1].meters()));
  double r = (posa? 1 : -1)*a.radius.meters();
  double t = (posb? 1 : -1)*b.radius.meters();
  double u = atan2(dif[0].meters(), dif[1].meters());
  double sol1 = asin((t - r)/C);
  double sol2 = M_PI - sol1;
  *outAngleA = Angle<double>::radians(sol1 - u);
  *outAngleB = Angle<double>::radians(sol2 - u);
}

namespace {
  Angle<double> makeCCW(Angle<double> x, bool positive) {
    return x + Angle<double>::degrees(positive? 0 : 180);
  }
}

SynthBoatTrajectory::WayPt::Connection
  SynthBoatTrajectory::WayPt::makeConnection(const WayPt &a, bool posa,
                                             const WayPt &b, bool posb) {
  Angle<double> sol1, sol2;
  solveCircleTangentLine(a, posa, b, posb,
      &sol1, &sol2);

  SynthBoatTrajectory::WayPt::Connection cona(a, b, makeCCW(sol1, posa), makeCCW(sol1, posb));
  SynthBoatTrajectory::WayPt::Connection conb(a, b, makeCCW(sol2, posa), makeCCW(sol2, posb));

  if (cona.isValid(a, posa)) {
    return cona;
  }
  assert(conb.isValid(a, posb));
  return conb;
}






} /* namespace mmm */
