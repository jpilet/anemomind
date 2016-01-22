/** Generated on Fri Jan 22 2016 12:09:43 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/BitStream.h>

namespace PgnClasses {

  VesselHeading::VesselHeading() {
    reset();
  }

  VesselHeading::VesselHeading(const uint8_t *data, int lengthBytes) {
    BitStream src(data, lengthBytes);
    if (58 <= src.remainingBits()) {
      {auto x = src.getUnsigned(8); if (BitStream::isAvailable(x, 8)) {_sid = x;}}
      {auto x = src.getUnsigned(16); if (BitStream::isAvailable(x, 16)) {_heading = double(0.0001*x)*sail::Angle<double>::radians(1.0);}}
      {auto x = src.getSigned(16); if (BitStream::isAvailable(x, 16)) {_deviation = double(0.0001*x)*sail::Angle<double>::radians(1.0);}}
      {auto x = src.getSigned(16); if (BitStream::isAvailable(x, 16)) {_variation = double(0.0001*x)*sail::Angle<double>::radians(1.0);}}
      {auto x = src.getUnsigned(2); if (BitStream::isAvailable(x, 2)) {_reference = x;}}

      _valid = true;
    } else {
      reset();
    }
  }

  void VesselHeading::reset() {
    _valid = false;
  }

  Attitude::Attitude() {
    reset();
  }

  Attitude::Attitude(const uint8_t *data, int lengthBytes) {
    BitStream src(data, lengthBytes);
    if (56 <= src.remainingBits()) {
      {auto x = src.getUnsigned(8); if (BitStream::isAvailable(x, 8)) {_sid = x;}}
      // Skipping yaw
      src.advanceBits(16);
      // Skipping pitch
      src.advanceBits(16);
      // Skipping roll
      src.advanceBits(16);

      _valid = true;
    } else {
      reset();
    }
  }

  void Attitude::reset() {
    _valid = false;
  }

  Speed::Speed() {
    reset();
  }

  Speed::Speed(const uint8_t *data, int lengthBytes) {
    BitStream src(data, lengthBytes);
    if (44 <= src.remainingBits()) {
      {auto x = src.getUnsigned(8); if (BitStream::isAvailable(x, 8)) {_sid = x;}}
      {auto x = src.getUnsigned(16); if (BitStream::isAvailable(x, 16)) {_speedWaterReferenced = double(0.01*x)*sail::Velocity<double>::metersPerSecond(1.0);}}
      {auto x = src.getUnsigned(16); if (BitStream::isAvailable(x, 16)) {_speedGroundReferenced = double(0.01*x)*sail::Velocity<double>::metersPerSecond(1.0);}}
      {auto x = src.getUnsigned(4); if (BitStream::isAvailable(x, 4)) {_speedWaterReferencedType = x;}}

      _valid = true;
    } else {
      reset();
    }
  }

  void Speed::reset() {
    _valid = false;
  }

  WindData::WindData() {
    reset();
  }

  WindData::WindData(const uint8_t *data, int lengthBytes) {
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
bool PgnVisitor::visit(int pgn, const uint8_t *data, int length) {
  switch(pgn) {
    case 127250: return apply(VesselHeading(data, length));
    case 127257: return apply(Attitude(data, length));
    case 128259: return apply(Speed(data, length));
    case 130306: return apply(WindData(data, length));
  }  // closes switch
  return false;
}

}