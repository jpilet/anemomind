/** Generated on Fri Jan 22 2016 14:08:12 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/N2kField.h>

namespace PgnClasses {

  VesselHeading::VesselHeading() {
    reset();
  }

  VesselHeading::VesselHeading(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (58 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _heading = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _deviation = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _variation = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _reference = src.getUnsigned(2, N2kField::Definedness::AlwaysDefined);
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
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (56 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
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
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (44 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _speedWaterReferenced = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _speedGroundReferenced = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _speedWaterReferencedType = src.getUnsigned(4, N2kField::Definedness::AlwaysDefined);
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
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (43 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _windSpeed = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _windAngle = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _reference = src.getUnsigned(3, N2kField::Definedness::AlwaysDefined);
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