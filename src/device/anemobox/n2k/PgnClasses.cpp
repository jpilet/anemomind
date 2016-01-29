/** Generated on Fri Jan 29 2016 12:19:28 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/N2kField.h>

namespace PgnClasses {

  IsoTransportProtocolDataTransfer::IsoTransportProtocolDataTransfer() {
    reset();
  }

  IsoTransportProtocolDataTransfer::IsoTransportProtocolDataTransfer(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _data = src.readBytes(56);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolDataTransfer::reset() {
    _valid = false;
  }

  IsoTransportProtocolConnectionManagementRequestToSend::IsoTransportProtocolConnectionManagementRequestToSend() {
    reset();
  }

  IsoTransportProtocolConnectionManagementRequestToSend::IsoTransportProtocolConnectionManagementRequestToSend(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _messageSize = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _packets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _packetsReply = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _pgn = src.getUnsigned(24, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementRequestToSend::reset() {
    _valid = false;
  }

  IsoTransportProtocolConnectionManagementClearToSend::IsoTransportProtocolConnectionManagementClearToSend() {
    reset();
  }

  IsoTransportProtocolConnectionManagementClearToSend::IsoTransportProtocolConnectionManagementClearToSend(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _maxPackets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _nextSid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(16);
      _pgn = src.getUnsigned(24, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementClearToSend::reset() {
    _valid = false;
  }

  IsoTransportProtocolConnectionManagementEndOfMessage::IsoTransportProtocolConnectionManagementEndOfMessage() {
    reset();
  }

  IsoTransportProtocolConnectionManagementEndOfMessage::IsoTransportProtocolConnectionManagementEndOfMessage(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _totalMessageSize = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _totalNumberOfPacketsReceived = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(8);
      _pgn = src.getUnsigned(24, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementEndOfMessage::reset() {
    _valid = false;
  }

  IsoTransportProtocolConnectionManagementBroadcastAnnounce::IsoTransportProtocolConnectionManagementBroadcastAnnounce() {
    reset();
  }

  IsoTransportProtocolConnectionManagementBroadcastAnnounce::IsoTransportProtocolConnectionManagementBroadcastAnnounce(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _messageSize = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _packets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(8);
      _pgn = src.getUnsigned(24, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementBroadcastAnnounce::reset() {
    _valid = false;
  }

  IsoTransportProtocolConnectionManagementAbort::IsoTransportProtocolConnectionManagementAbort() {
    reset();
  }

  IsoTransportProtocolConnectionManagementAbort::IsoTransportProtocolConnectionManagementAbort(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (56 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _reason = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(16);
      _pgn = src.getUnsigned(24, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementAbort::reset() {
    _valid = false;
  }

  SystemTime::SystemTime() {
    reset();
  }

  SystemTime::SystemTime(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _source = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5}).cast<Source>();
      // Skipping reserved
      src.advanceBits(4);
      _date = src.getPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0);
      _time = src.getPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0);
      _valid = _source.defined();
    } else {
      reset();
    }
  }

  void SystemTime::reset() {
    _valid = false;
  }

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
      _reference = src.getUnsignedInSet(2, {0, 1}).cast<Reference>();
      _valid = _reference.defined();
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
      _speedWaterReferencedType = src.getUnsignedInSet(4, {0, 1, 2, 3, 4}).cast<SpeedWaterReferencedType>();
      _valid = _speedWaterReferencedType.defined();
    } else {
      reset();
    }
  }

  void Speed::reset() {
    _valid = false;
  }

  PositionRapidUpdate::PositionRapidUpdate() {
    reset();
  }

  PositionRapidUpdate::PositionRapidUpdate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _latitude = src.getPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0);
      _longitude = src.getPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0);
      _valid = true;
    } else {
      reset();
    }
  }

  void PositionRapidUpdate::reset() {
    _valid = false;
  }

  CogSogRapidUpdate::CogSogRapidUpdate() {
    reset();
  }

  CogSogRapidUpdate::CogSogRapidUpdate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _cogReference = src.getUnsignedInSet(2, {0, 1}).cast<CogReference>();
      // Skipping reserved
      src.advanceBits(6);
      _cog = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _sog = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      // Skipping reserved
      src.advanceBits(16);
      _valid = _cogReference.defined();
    } else {
      reset();
    }
  }

  void CogSogRapidUpdate::reset() {
    _valid = false;
  }

  GnssPositionData::GnssPositionData() {
    reset();
  }

  GnssPositionData::GnssPositionData(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (360 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _date = src.getPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0);
      _time = src.getPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0);
      _latitude = src.getPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0);
      _longitude = src.getPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0);
      _altitude = src.getPhysicalQuantity(true, 1e-06, sail::Length<double>::meters(1.0), 64, 0);
      _gnssType = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5, 6, 7, 8}).cast<GnssType>();
      _method = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5, 6, 7, 8}).cast<Method>();
      _integrity = src.getUnsignedInSet(2, {0, 1, 2}).cast<Integrity>();
      // Skipping reserved
      src.advanceBits(6);
      _numberOfSvs = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _hdop = src.getSigned(16, 0, N2kField::Definedness::AlwaysDefined);
      _pdop = src.getSigned(16, 0, N2kField::Definedness::AlwaysDefined);
      _geoidalSeparation = src.getPhysicalQuantity(true, 0.01, sail::Length<double>::meters(1.0), 16, 0);
      _referenceStations = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _referenceStationType = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5, 6, 7, 8}).cast<ReferenceStationType>();
      // Skipping referenceStationId
      src.advanceBits(12);
      _ageOfDgnssCorrections = src.getPhysicalQuantity(false, 0.01, sail::Duration<double>::seconds(1.0), 16, 0);
      _valid = _gnssType.defined() && _method.defined() && _integrity.defined() && _referenceStationType.defined();
    } else {
      reset();
    }
  }

  void GnssPositionData::reset() {
    _valid = false;
  }

  TimeDate::TimeDate() {
    reset();
  }

  TimeDate::TimeDate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      _date = src.getPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0);
      _time = src.getPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0);
      _localOffset = src.getPhysicalQuantity(true, 1, sail::Duration<double>::minutes(1.0), 16, 0);
      _valid = true;
    } else {
      reset();
    }
  }

  void TimeDate::reset() {
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
      _reference = src.getUnsignedInSet(3, {0, 1, 2, 3, 4}).cast<Reference>();
      _valid = _reference.defined();
    } else {
      reset();
    }
  }

  void WindData::reset() {
    _valid = false;
  }

  DirectionData::DirectionData() {
    reset();
  }

  DirectionData::DirectionData(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (112 <= src.remainingBits()) {
      _dataMode = src.getUnsignedInSet(4, {0, 1, 2, 3, 4}).cast<DataMode>();
      _cogReference = src.getUnsignedInSet(2, {0, 1}).cast<CogReference>();
      // Skipping reserved
      src.advanceBits(2);
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _cog = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _sog = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _heading = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _speedThroughWater = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _set = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _drift = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _valid = _dataMode.defined() && _cogReference.defined();
    } else {
      reset();
    }
  }

  void DirectionData::reset() {
    _valid = false;
  }

  SimnetEventCommandApCommand::SimnetEventCommandApCommand() {
    reset();
  }

  SimnetEventCommandApCommand::SimnetEventCommandApCommand(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (96 <= src.remainingBits()) {
      _manufacturerCode = src.getUnsigned(11, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(2);
      _industryCode = src.getUnsigned(3, N2kField::Definedness::AlwaysDefined);
      _proprietaryId = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _b = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _controllingDevice = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _event = src.getUnsignedInSet(16, {10, 13, 15, 26, 6, 9}).cast<Event>();
      _direction = src.getUnsignedInSet(8, {2, 3, 4, 5}).cast<Direction>();
      _angle = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _g = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _valid = _event.defined() && _direction.defined();
    } else {
      reset();
    }
  }

  void SimnetEventCommandApCommand::reset() {
    _valid = false;
  }

  SimnetEventCommandAlarm::SimnetEventCommandAlarm() {
    reset();
  }

  SimnetEventCommandAlarm::SimnetEventCommandAlarm(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (96 <= src.remainingBits()) {
      _manufacturerCode = src.getUnsigned(11, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(2);
      _industryCode = src.getUnsigned(3, N2kField::Definedness::AlwaysDefined);
      _a = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _proprietaryId = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _c = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _alarm = src.getUnsignedInSet(16, {56, 57}).cast<Alarm>();
      _messageId = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _f = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _g = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _valid = _alarm.defined();
    } else {
      reset();
    }
  }

  void SimnetEventCommandAlarm::reset() {
    _valid = false;
  }

  SimnetEventCommandUnknown::SimnetEventCommandUnknown() {
    reset();
  }

  SimnetEventCommandUnknown::SimnetEventCommandUnknown(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (96 <= src.remainingBits()) {
      _manufacturerCode = src.getUnsigned(11, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(2);
      _industryCode = src.getUnsigned(3, N2kField::Definedness::AlwaysDefined);
      _a = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _proprietaryId = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _b = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _c = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _d = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _e = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void SimnetEventCommandUnknown::reset() {
    _valid = false;
  }

bool PgnVisitor::visit(const std::string& src, int pgn, const uint8_t *data, int length) {
  switch(pgn) {
    case 60160: return apply(src, IsoTransportProtocolDataTransfer(data, length));
    case 60416: {
      BitStream dispatchStream(data, length);
      auto dispatchCode0 = dispatchStream.getUnsigned(8);
      switch(dispatchCode0) {
        case 16: return apply(src, IsoTransportProtocolConnectionManagementRequestToSend(data, length));
        case 17: return apply(src, IsoTransportProtocolConnectionManagementClearToSend(data, length));
        case 19: return apply(src, IsoTransportProtocolConnectionManagementEndOfMessage(data, length));
        case 32: return apply(src, IsoTransportProtocolConnectionManagementBroadcastAnnounce(data, length));
        case 255: return apply(src, IsoTransportProtocolConnectionManagementAbort(data, length));
        default: return false;
      };
      break;
    }
    case 126992: return apply(src, SystemTime(data, length));
    case 127250: return apply(src, VesselHeading(data, length));
    case 127257: return apply(src, Attitude(data, length));
    case 128259: return apply(src, Speed(data, length));
    case 129025: return apply(src, PositionRapidUpdate(data, length));
    case 129026: return apply(src, CogSogRapidUpdate(data, length));
    case 129029: return apply(src, GnssPositionData(data, length));
    case 129033: return apply(src, TimeDate(data, length));
    case 130306: return apply(src, WindData(data, length));
    case 130577: return apply(src, DirectionData(data, length));
    case 130850: {
      BitStream dispatchStream(data, length);
      auto dispatchCode0 = dispatchStream.getUnsigned(11);
      switch(dispatchCode0) {
        case 1857: {
          switch(getDispatchCodeFor130850_1857(data, length)) {
            case PgnVariant130850::TypeSimnetEventCommandApCommand: return apply(src, SimnetEventCommandApCommand(data, length));
            case PgnVariant130850::TypeSimnetEventCommandAlarm: return apply(src, SimnetEventCommandAlarm(data, length));
            case PgnVariant130850::TypeSimnetEventCommandUnknown: return apply(src, SimnetEventCommandUnknown(data, length));
            default: return false;
          };
          break;
        }
        default: return false;
      };
      break;
    }
    default: return false;
  };
}
}