/** Generated on Thu Jan 21 2016 20:41:35 GMT+0100 (CET) using 
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
      _sid = src.getOptionalUnsigned(8);
      src.getOptionalUnsigned(16).then([&](uint64_t x) {
        _heading = (0.0001*x)*sail::Angle<double>::radians(1.0);
      });
      src.getOptionalSigned(16).then([&](int64_t x) {
        _deviation = (0.0001*x)*sail::Angle<double>::radians(1.0);
      });
      src.getOptionalSigned(16).then([&](int64_t x) {
        _variation = (0.0001*x)*sail::Angle<double>::radians(1.0);
      });
      _reference = src.getOptionalUnsigned(2);
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
      _sid = src.getOptionalUnsigned(8);      // Skipping yaw
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
      _sid = src.getOptionalUnsigned(8);
      src.getOptionalUnsigned(16).then([&](uint64_t x) {
        _speedWaterReferenced = (0.01*x)*sail::Velocity<double>::metersPerSecond(1.0);
      });
      src.getOptionalUnsigned(16).then([&](uint64_t x) {
        _speedGroundReferenced = (0.01*x)*sail::Velocity<double>::metersPerSecond(1.0);
      });
      _speedWaterReferencedType = src.getOptionalUnsigned(4);
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
      _sid = src.getOptionalUnsigned(8);
      src.getOptionalUnsigned(16).then([&](uint64_t x) {
        _windSpeed = (0.01*x)*sail::Velocity<double>::metersPerSecond(1.0);
      });
      src.getOptionalUnsigned(16).then([&](uint64_t x) {
        _windAngle = (0.0001*x)*sail::Angle<double>::radians(1.0);
      });
      _reference = src.getOptionalUnsigned(3);
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