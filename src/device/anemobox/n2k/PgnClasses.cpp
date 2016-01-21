/** Generated on Thu Jan 21 2016 12:13:55 GMT+0100 (CET) using 
 *
 *  node codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml /home/jonas/programmering/sailsmart/src/device/anemobox/n2k
 *
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/BitStream.h>

namespace PgnClasses {
  WindData::WindData(BitStream *src) : _valid(false) {
    if (43 <= src->remainingBits()) {
      _sid = src->getSigned(8);
      _windSpeed = double(src->getSigned(16)*0.01)*sail::Velocity<double>::metersPerSecond(1.0);
      _windAngle = double(src->getSigned(16)*0.0001)*sail::Angle<double>::radians(1.0);
      _reference = src->getSigned(3);
    }
  }
}