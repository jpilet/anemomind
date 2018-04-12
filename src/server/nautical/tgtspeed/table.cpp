/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/tgtspeed/table.h>
#include <server/common/string.h>
#include <server/transducers/Transducer.h>

namespace sail {

Array<Angle<double> > getNorthSailsAngles() {
  const int count = 17;
  double anglesDeg[count] = {32, 36, 40, 45, 52,
                       60, 70, 80, 90, 100,
                       110, 120, 135, 150,
                       160, 170, 180};
  return transduce(
      Arrayd(count, anglesDeg),
      trMap([&](double x) {
        return Angle<double>::degrees(x);
      }),
      IntoArray<Angle<double>>());
}

Array<Velocity<double> > getNorthSailsSpeeds() {
  const int count = 9;
  double speedsKnots[count] = {4, 6, 8, 10, 12, 14, 16, 20, 25};
  auto toVel = [&](double x) {
        return Velocity<double>::knots(x);
      };
  return transduce(
      Arrayd(count, speedsKnots),
      trMap(toVel),
      IntoArray<Velocity<double>>());
}

void outputValue(double x, std::ostream *dst) {
  *dst << stringFormat("%17.2f", x);
}

void outputTWSHeader(Array<Velocity<double> > tws, std::ostream *dst) {
  *dst << stringFormat("TWS (knots) =    ");
  for (int i = 0; i < tws.size(); i++) {
    outputValue(tws[i].knots(), dst);
  }
  *dst << "\n";
}

void outputTWALabel(Angle<double> degs, std::ostream *dst) {
  *dst << stringFormat("TWA=%7.2f deg  ", degs);
}

}
