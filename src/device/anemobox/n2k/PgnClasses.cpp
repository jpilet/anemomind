/** Generated on Fri Jan 19 2018 13:27:28 GMT+0100 (CET) using 
 *
 *     /usr/local/bin/node /Users/jonas/prog/anemomind/src/device/anemobox/n2k/codegen/index.js /Users/jonas/prog/canboat/analyzer/pgns.xml
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
    // Warning: PGN 60160 (ISO Transport Protocol, Data Transfer) has 1 repeating fields that are not handled.
    if (8 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolDataTransfer::reset() {
    _valid = false;
  }
  std::vector<uint8_t> IsoTransportProtocolDataTransfer::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementRequestToSend::IsoTransportProtocolConnectionManagementRequestToSend() {
    reset();
  }

  IsoTransportProtocolConnectionManagementRequestToSend::IsoTransportProtocolConnectionManagementRequestToSend(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    // Warning: PGN 60416 (ISO Transport Protocol, Connection Management - Request To Send) has 1 repeating fields that are not handled.
    if (40 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _messageSize = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _packets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _packetsReply = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementRequestToSend::reset() {
    _valid = false;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementRequestToSend::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _groupFunctionCode);
      dst.pushUnsigned(16, _messageSize);
      dst.pushUnsigned(8, _packets);
      dst.pushUnsigned(8, _packetsReply);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementClearToSend::IsoTransportProtocolConnectionManagementClearToSend() {
    reset();
  }

  IsoTransportProtocolConnectionManagementClearToSend::IsoTransportProtocolConnectionManagementClearToSend(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    // Warning: PGN 60416 (ISO Transport Protocol, Connection Management - Clear To Send) has 1 repeating fields that are not handled.
    if (40 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _maxPackets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _nextSid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(16);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementClearToSend::reset() {
    _valid = false;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementClearToSend::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _groupFunctionCode);
      dst.pushUnsigned(8, _maxPackets);
      dst.pushUnsigned(8, _nextSid);
      dst.fillBits(16, true); // TODO: Can we safely do this? The field name is 'Reserved'
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementEndOfMessage::IsoTransportProtocolConnectionManagementEndOfMessage() {
    reset();
  }

  IsoTransportProtocolConnectionManagementEndOfMessage::IsoTransportProtocolConnectionManagementEndOfMessage(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    // Warning: PGN 60416 (ISO Transport Protocol, Connection Management - End Of Message) has 1 repeating fields that are not handled.
    if (40 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _totalMessageSize = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _totalNumberOfPacketsReceived = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(8);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementEndOfMessage::reset() {
    _valid = false;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementEndOfMessage::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _groupFunctionCode);
      dst.pushUnsigned(16, _totalMessageSize);
      dst.pushUnsigned(8, _totalNumberOfPacketsReceived);
      dst.fillBits(8, true); // TODO: Can we safely do this? The field name is 'Reserved'
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementBroadcastAnnounce::IsoTransportProtocolConnectionManagementBroadcastAnnounce() {
    reset();
  }

  IsoTransportProtocolConnectionManagementBroadcastAnnounce::IsoTransportProtocolConnectionManagementBroadcastAnnounce(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    // Warning: PGN 60416 (ISO Transport Protocol, Connection Management - Broadcast Announce) has 1 repeating fields that are not handled.
    if (40 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _messageSize = src.getUnsigned(16, N2kField::Definedness::AlwaysDefined);
      _packets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(8);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementBroadcastAnnounce::reset() {
    _valid = false;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementBroadcastAnnounce::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _groupFunctionCode);
      dst.pushUnsigned(16, _messageSize);
      dst.pushUnsigned(8, _packets);
      dst.fillBits(8, true); // TODO: Can we safely do this? The field name is 'Reserved'
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementAbort::IsoTransportProtocolConnectionManagementAbort() {
    reset();
  }

  IsoTransportProtocolConnectionManagementAbort::IsoTransportProtocolConnectionManagementAbort(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    // Warning: PGN 60416 (ISO Transport Protocol, Connection Management - Abort) has 1 repeating fields that are not handled.
    if (32 <= src.remainingBits()) {
      _groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _reason = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(16);
      _valid = true;
    } else {
      reset();
    }
  }

  void IsoTransportProtocolConnectionManagementAbort::reset() {
    _valid = false;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementAbort::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _groupFunctionCode);
      dst.pushUnsigned(8, _reason);
      dst.fillBits(16, true); // TODO: Can we safely do this? The field name is 'Reserved'
    return dst.moveData();
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
  std::vector<uint8_t> SystemTime::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushUnsigned(4, _source.cast<uint64_t>());
      dst.fillBits(4, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0, _date);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0, _time);
    dst.fillUpToLength(64, true);
    return dst.moveData();
  }

  Rudder::Rudder() {
    reset();
  }

  Rudder::Rudder(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (48 <= src.remainingBits()) {
      _instance = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _directionOrder = src.getUnsigned(2, N2kField::Definedness::AlwaysDefined);
      // Skipping reserved
      src.advanceBits(6);
      _angleOrder = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _position = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _valid = true;
    } else {
      reset();
    }
  }

  void Rudder::reset() {
    _valid = false;
  }
  std::vector<uint8_t> Rudder::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _instance);
      dst.pushUnsigned(2, _directionOrder);
      dst.fillBits(6, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _angleOrder);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _position);
    dst.fillUpToLength(64, true);
    return dst.moveData();
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
  std::vector<uint8_t> VesselHeading::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _heading);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _deviation);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _variation);
      dst.pushUnsigned(2, _reference.cast<uint64_t>());
    dst.fillUpToLength(64, true);
    return dst.moveData();
  }

  RateOfTurn::RateOfTurn() {
    reset();
  }

  RateOfTurn::RateOfTurn(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (40 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _rate = src.getPhysicalQuantity(true, 3.125e-08, (sail::Angle<double>::radians(1.0)/sail::Duration<double>::seconds(1.0)), 32, 0);
      _valid = true;
    } else {
      reset();
    }
  }

  void RateOfTurn::reset() {
    _valid = false;
  }
  std::vector<uint8_t> RateOfTurn::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(true, 3.125e-08, (sail::Angle<double>::radians(1.0)/sail::Duration<double>::seconds(1.0)), 32, 0, _rate);
    dst.fillUpToLength(40, true);
    return dst.moveData();
  }

  Attitude::Attitude() {
    reset();
  }

  Attitude::Attitude(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (56 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _yaw = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _pitch = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _roll = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      _valid = true;
    } else {
      reset();
    }
  }

  void Attitude::reset() {
    _valid = false;
  }
  std::vector<uint8_t> Attitude::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _yaw);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _pitch);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _roll);
    dst.fillUpToLength(56, true);
    return dst.moveData();
  }

  Speed::Speed() {
    reset();
  }

  Speed::Speed(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (52 <= src.remainingBits()) {
      _sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      _speedWaterReferenced = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _speedGroundReferenced = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      _speedWaterReferencedType = src.getUnsignedInSet(8, {0, 1, 2, 3, 4}).cast<SpeedWaterReferencedType>();
      _speedDirection = src.getUnsigned(4, N2kField::Definedness::AlwaysDefined);
      _valid = _speedWaterReferencedType.defined();
    } else {
      reset();
    }
  }

  void Speed::reset() {
    _valid = false;
  }
  std::vector<uint8_t> Speed::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _speedWaterReferenced);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _speedGroundReferenced);
      dst.pushUnsigned(8, _speedWaterReferencedType.cast<uint64_t>());
      dst.pushUnsigned(4, _speedDirection);
    dst.fillUpToLength(48, true);
    return dst.moveData();
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
  std::vector<uint8_t> PositionRapidUpdate::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0, _latitude);
      dst.pushPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0, _longitude);
    dst.fillUpToLength(64, true);
    return dst.moveData();
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
  std::vector<uint8_t> CogSogRapidUpdate::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushUnsigned(2, _cogReference.cast<uint64_t>());
      dst.fillBits(6, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _cog);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _sog);
      dst.fillBits(16, true); // TODO: Can we safely do this? The field name is 'Reserved'
    dst.fillUpToLength(64, true);
    return dst.moveData();
  }

  GnssPositionData::GnssPositionData() {
    reset();
  }

  GnssPositionData::GnssPositionData(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    // Warning: PGN 129029 (GNSS Position Data) has 3 repeating fields that are not handled.
    if (328 <= src.remainingBits()) {
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
      _valid = _gnssType.defined() && _method.defined() && _integrity.defined();
    } else {
      reset();
    }
  }

  void GnssPositionData::reset() {
    _valid = false;
  }
  std::vector<uint8_t> GnssPositionData::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0, _date);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0, _time);
      dst.pushPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0, _latitude);
      dst.pushPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0, _longitude);
      dst.pushPhysicalQuantity(true, 1e-06, sail::Length<double>::meters(1.0), 64, 0, _altitude);
      dst.pushUnsigned(4, _gnssType.cast<uint64_t>());
      dst.pushUnsigned(4, _method.cast<uint64_t>());
      dst.pushUnsigned(2, _integrity.cast<uint64_t>());
      dst.fillBits(6, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushUnsigned(8, _numberOfSvs);
      dst.pushSigned(16, 0, _hdop);
      dst.pushSigned(16, 0, _pdop);
      dst.pushPhysicalQuantity(true, 0.01, sail::Length<double>::meters(1.0), 16, 0, _geoidalSeparation);
      dst.pushUnsigned(8, _referenceStations);
    return dst.moveData();
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
  std::vector<uint8_t> TimeDate::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0, _date);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0, _time);
      dst.pushPhysicalQuantity(true, 1, sail::Duration<double>::minutes(1.0), 16, 0, _localOffset);
    dst.fillUpToLength(64, true);
    return dst.moveData();
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
  std::vector<uint8_t> WindData::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _windSpeed);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _windAngle);
      dst.pushUnsigned(3, _reference.cast<uint64_t>());
    dst.fillUpToLength(48, true);
    return dst.moveData();
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
  std::vector<uint8_t> DirectionData::encode() const {
    N2kField::N2kFieldOutputStream dst;
      dst.pushUnsigned(4, _dataMode.cast<uint64_t>());
      dst.pushUnsigned(2, _cogReference.cast<uint64_t>());
      dst.fillBits(2, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushUnsigned(8, _sid);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _cog);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _sog);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _heading);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _speedThroughWater);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, _set);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, _drift);
    dst.fillUpToLength(112, true);
    return dst.moveData();
  }

int pgnSize(int pgn) {
  switch(pgn) {
    case 127251: return 5;
    case 127257: return 7;
    case 128259: return 6;
    case 129029: return 51;
    case 130306: return 6;
    case 130577: return 14;
    default: return 8;
  }
}

void PgnVisitor::pushAndLinkPacket(const CanPacket& packet) {
  if (packet.data.size() == 8 && pgnSize(packet.pgn) > 8) {
    add(packet);
  } else {
    visit(packet);
  }
}

void PgnVisitor::fullPacketReceived(const CanPacket& fullPacket) {
  visit(fullPacket);
}

bool PgnVisitor::visit(const CanPacket &packet) {
  switch(packet.pgn) {
    case 60160: return apply(packet, IsoTransportProtocolDataTransfer(&(packet.data[0]), packet.data.size()));
    case 60416: {
      BitStream dispatchStream(&(packet.data)[0], packet.data.size());
      auto dispatchCode0 = dispatchStream.getUnsigned(8);
      switch(dispatchCode0) {
        case 16: return apply(packet, IsoTransportProtocolConnectionManagementRequestToSend(&(packet.data[0]), packet.data.size()));
        case 17: return apply(packet, IsoTransportProtocolConnectionManagementClearToSend(&(packet.data[0]), packet.data.size()));
        case 19: return apply(packet, IsoTransportProtocolConnectionManagementEndOfMessage(&(packet.data[0]), packet.data.size()));
        case 32: return apply(packet, IsoTransportProtocolConnectionManagementBroadcastAnnounce(&(packet.data[0]), packet.data.size()));
        case 255: return apply(packet, IsoTransportProtocolConnectionManagementAbort(&(packet.data[0]), packet.data.size()));
        default: return false;
      };
      break;
    }
    case 126992: return apply(packet, SystemTime(&(packet.data[0]), packet.data.size()));
    case 127245: return apply(packet, Rudder(&(packet.data[0]), packet.data.size()));
    case 127250: return apply(packet, VesselHeading(&(packet.data[0]), packet.data.size()));
    case 127251: return apply(packet, RateOfTurn(&(packet.data[0]), packet.data.size()));
    case 127257: return apply(packet, Attitude(&(packet.data[0]), packet.data.size()));
    case 128259: return apply(packet, Speed(&(packet.data[0]), packet.data.size()));
    case 129025: return apply(packet, PositionRapidUpdate(&(packet.data[0]), packet.data.size()));
    case 129026: return apply(packet, CogSogRapidUpdate(&(packet.data[0]), packet.data.size()));
    case 129029: return apply(packet, GnssPositionData(&(packet.data[0]), packet.data.size()));
    case 129033: return apply(packet, TimeDate(&(packet.data[0]), packet.data.size()));
    case 130306: return apply(packet, WindData(&(packet.data[0]), packet.data.size()));
    case 130577: return apply(packet, DirectionData(&(packet.data[0]), packet.data.size()));
    default: return false;
  };
  return false;
}
}