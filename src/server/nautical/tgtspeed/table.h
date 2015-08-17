/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_TGTSPEED_TABLE_H_
#define SERVER_NAUTICAL_TGTSPEED_TABLE_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

Array<Angle<double> > getNorthSailsAngles();
Array<Velocity<double> > getNorthSailsSpeeds();
void outputValue(double x, std::ostream *dst);
void outputTWSHeader(Array<Velocity<double> > tws, std::ostream *dst);
void outputTWALabel(Angle<double> degs, std::ostream *dst);

}

#endif /* SERVER_NAUTICAL_TGTSPEED_TABLE_H_ */
