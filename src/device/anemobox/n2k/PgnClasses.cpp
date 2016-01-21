/** Generated on Thu Jan 21 2016 14:14:50 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/BitStream.h>

namespace PgnClasses {

  WindData::WindData() {
    reset();
  }

  WindData::WindData(BitStream *src) {
    if (43 <= src->remainingBits()) {
      _sid = src->getSigned(8);
      _windSpeed = double(src->getSigned(16)*0.01)*sail::Velocity<double>::metersPerSecond(1.0);
      _windAngle = double(src->getSigned(16)*0.0001)*sail::Angle<double>::radians(1.0);
      _reference = src->getSigned(3);
      _valid = true;
    } else {
      reset();
    }
  }

  void WindData::reset() {
    _valid = false;
    _sid = 0;
    _windSpeed = sail::Velocity<double>();
    _windAngle = sail::Angle<double>();
    _reference = 0;
  }
}