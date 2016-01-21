/** Generated on Thu Jan 21 2016 16:27:49 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/BitStream.h>

namespace PgnClasses {

  WindData::WindData() {
    reset();
  }

  WindData::WindData(uint8_t *data, int lengthBytes) {
    BitStream src(data, lengthBytes);
    if (43 <= src.remainingBits()) {
      {auto x = src.getUnsigned(8); if (BitStream::isAvailable(x, 8)) {_sid = x;}}
      {auto x = src.getUnsigned(16); if (BitStream::isAvailable(x, 16)) {_windSpeed = double(0.01*x)*sail::Velocity<double>::metersPerSecond(1.0);}}
      {auto x = src.getUnsigned(16); if (BitStream::isAvailable(x, 16)) {_windAngle = double(0.0001*x)*sail::Angle<double>::radians(1.0);}}
      {auto x = src.getUnsigned(3); if (BitStream::isAvailable(x, 3)) {_reference = x;}}
      _valid = true;
    } else {
      reset();
    }
  }

  void WindData::reset() {
    _valid = false;
  }
}