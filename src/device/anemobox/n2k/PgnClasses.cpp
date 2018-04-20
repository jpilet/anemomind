/** Generated on Fri Apr 20 2018 15:14:02 GMT+0200 (CEST) using 
 *
 *     /usr/local/bin/node /Users/jonas/prog/anemomind/src/device/anemobox/n2k/codegen/index.js /Users/jonas/prog/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#include "PgnClasses.h"

#include <device/anemobox/n2k/N2kField.h>
#include<server/common/logging.h>
#include <iostream>

namespace PgnClasses {

  IsoTransportProtocolDataTransfer::IsoTransportProtocolDataTransfer() {
  }

  IsoTransportProtocolDataTransfer::IsoTransportProtocolDataTransfer(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (8 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      while (56 <= src.remainingBits()) {
        auto l_data = src.readBytes(56);
        repeating.push_back({l_data});
      }
    }
  }
  bool IsoTransportProtocolDataTransfer::hasSomeData() const {
    return 
         sid.defined()
    ;
  }
  bool IsoTransportProtocolDataTransfer::hasAllData() const {
    return 
         sid.defined()
    ;
  }
  bool IsoTransportProtocolDataTransfer::valid() const {
    return true
    ;
  }
  std::vector<uint8_t> IsoTransportProtocolDataTransfer::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode IsoTransportProtocolDataTransfer";
      return {};
    }
      dst.pushUnsigned(8, sid);
    for (const auto& x: repeating) {
      dst.pushBytes(56, x.data);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementRequestToSend::IsoTransportProtocolConnectionManagementRequestToSend() {
  }

  IsoTransportProtocolConnectionManagementRequestToSend::IsoTransportProtocolConnectionManagementRequestToSend(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (40 <= src.remainingBits()) {
      groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      messageSize = src.getUnsigned(16, N2kField::Definedness::MaybeUndefined);
      packets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      packetsReply = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      while (24 <= src.remainingBits()) {
        auto l_pgn = src.getUnsigned(24, N2kField::Definedness::MaybeUndefined);
        repeating.push_back({l_pgn});
      }
    }
  }
  bool IsoTransportProtocolConnectionManagementRequestToSend::hasSomeData() const {
    return 
         groupFunctionCode.defined()
      || messageSize.defined()
      || packets.defined()
      || packetsReply.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementRequestToSend::hasAllData() const {
    return 
         groupFunctionCode.defined()
      && messageSize.defined()
      && packets.defined()
      && packetsReply.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementRequestToSend::valid() const {
    return true
       && groupFunctionCode.defined() && groupFunctionCode.get() == 16
    ;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementRequestToSend::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode IsoTransportProtocolConnectionManagementRequestToSend";
      return {};
    }
      dst.pushUnsigned(8, groupFunctionCode);
      dst.pushUnsigned(16, messageSize);
      dst.pushUnsigned(8, packets);
      dst.pushUnsigned(8, packetsReply);
    for (const auto& x: repeating) {
      dst.pushUnsigned(24, x.pgn);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementClearToSend::IsoTransportProtocolConnectionManagementClearToSend() {
  }

  IsoTransportProtocolConnectionManagementClearToSend::IsoTransportProtocolConnectionManagementClearToSend(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (40 <= src.remainingBits()) {
      groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      maxPackets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      nextSid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
        // Skipping reserved
        src.advanceBits(16);
      while (24 <= src.remainingBits()) {
        auto l_pgn = src.getUnsigned(24, N2kField::Definedness::MaybeUndefined);
        repeating.push_back({l_pgn});
      }
    }
  }
  bool IsoTransportProtocolConnectionManagementClearToSend::hasSomeData() const {
    return 
         groupFunctionCode.defined()
      || maxPackets.defined()
      || nextSid.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementClearToSend::hasAllData() const {
    return 
         groupFunctionCode.defined()
      && maxPackets.defined()
      && nextSid.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementClearToSend::valid() const {
    return true
       && groupFunctionCode.defined() && groupFunctionCode.get() == 17
    ;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementClearToSend::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode IsoTransportProtocolConnectionManagementClearToSend";
      return {};
    }
      dst.pushUnsigned(8, groupFunctionCode);
      dst.pushUnsigned(8, maxPackets);
      dst.pushUnsigned(8, nextSid);
      dst.fillBits(16, true); // TODO: Can we safely do this? The field name is 'Reserved'
    for (const auto& x: repeating) {
      dst.pushUnsigned(24, x.pgn);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementEndOfMessage::IsoTransportProtocolConnectionManagementEndOfMessage() {
  }

  IsoTransportProtocolConnectionManagementEndOfMessage::IsoTransportProtocolConnectionManagementEndOfMessage(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (40 <= src.remainingBits()) {
      groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      totalMessageSize = src.getUnsigned(16, N2kField::Definedness::MaybeUndefined);
      totalNumberOfPacketsReceived = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
        // Skipping reserved
        src.advanceBits(8);
      while (24 <= src.remainingBits()) {
        auto l_pgn = src.getUnsigned(24, N2kField::Definedness::MaybeUndefined);
        repeating.push_back({l_pgn});
      }
    }
  }
  bool IsoTransportProtocolConnectionManagementEndOfMessage::hasSomeData() const {
    return 
         groupFunctionCode.defined()
      || totalMessageSize.defined()
      || totalNumberOfPacketsReceived.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementEndOfMessage::hasAllData() const {
    return 
         groupFunctionCode.defined()
      && totalMessageSize.defined()
      && totalNumberOfPacketsReceived.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementEndOfMessage::valid() const {
    return true
       && groupFunctionCode.defined() && groupFunctionCode.get() == 19
    ;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementEndOfMessage::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode IsoTransportProtocolConnectionManagementEndOfMessage";
      return {};
    }
      dst.pushUnsigned(8, groupFunctionCode);
      dst.pushUnsigned(16, totalMessageSize);
      dst.pushUnsigned(8, totalNumberOfPacketsReceived);
      dst.fillBits(8, true); // TODO: Can we safely do this? The field name is 'Reserved'
    for (const auto& x: repeating) {
      dst.pushUnsigned(24, x.pgn);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementBroadcastAnnounce::IsoTransportProtocolConnectionManagementBroadcastAnnounce() {
  }

  IsoTransportProtocolConnectionManagementBroadcastAnnounce::IsoTransportProtocolConnectionManagementBroadcastAnnounce(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (40 <= src.remainingBits()) {
      groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      messageSize = src.getUnsigned(16, N2kField::Definedness::MaybeUndefined);
      packets = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
        // Skipping reserved
        src.advanceBits(8);
      while (24 <= src.remainingBits()) {
        auto l_pgn = src.getUnsigned(24, N2kField::Definedness::MaybeUndefined);
        repeating.push_back({l_pgn});
      }
    }
  }
  bool IsoTransportProtocolConnectionManagementBroadcastAnnounce::hasSomeData() const {
    return 
         groupFunctionCode.defined()
      || messageSize.defined()
      || packets.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementBroadcastAnnounce::hasAllData() const {
    return 
         groupFunctionCode.defined()
      && messageSize.defined()
      && packets.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementBroadcastAnnounce::valid() const {
    return true
       && groupFunctionCode.defined() && groupFunctionCode.get() == 32
    ;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementBroadcastAnnounce::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode IsoTransportProtocolConnectionManagementBroadcastAnnounce";
      return {};
    }
      dst.pushUnsigned(8, groupFunctionCode);
      dst.pushUnsigned(16, messageSize);
      dst.pushUnsigned(8, packets);
      dst.fillBits(8, true); // TODO: Can we safely do this? The field name is 'Reserved'
    for (const auto& x: repeating) {
      dst.pushUnsigned(24, x.pgn);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  IsoTransportProtocolConnectionManagementAbort::IsoTransportProtocolConnectionManagementAbort() {
  }

  IsoTransportProtocolConnectionManagementAbort::IsoTransportProtocolConnectionManagementAbort(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (32 <= src.remainingBits()) {
      groupFunctionCode = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      reason = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
        // Skipping reserved
        src.advanceBits(16);
      while (24 <= src.remainingBits()) {
        auto l_pgn = src.getUnsigned(24, N2kField::Definedness::MaybeUndefined);
        repeating.push_back({l_pgn});
      }
    }
  }
  bool IsoTransportProtocolConnectionManagementAbort::hasSomeData() const {
    return 
         groupFunctionCode.defined()
      || reason.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementAbort::hasAllData() const {
    return 
         groupFunctionCode.defined()
      && reason.defined()
    ;
  }
  bool IsoTransportProtocolConnectionManagementAbort::valid() const {
    return true
       && groupFunctionCode.defined() && groupFunctionCode.get() == 255
    ;
  }
  std::vector<uint8_t> IsoTransportProtocolConnectionManagementAbort::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode IsoTransportProtocolConnectionManagementAbort";
      return {};
    }
      dst.pushUnsigned(8, groupFunctionCode);
      dst.pushUnsigned(8, reason);
      dst.fillBits(16, true); // TODO: Can we safely do this? The field name is 'Reserved'
    for (const auto& x: repeating) {
      dst.pushUnsigned(24, x.pgn);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  SystemTime::SystemTime() {
  }

  SystemTime::SystemTime(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      source = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5}).cast<Source>();
        // Skipping reserved
        src.advanceBits(4);
      date = src.getPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0);
      time = src.getPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0);
    // No repeating fields.
    }
  }
  bool SystemTime::hasSomeData() const {
    return 
         sid.defined()
      || source.defined()
      || date.defined()
      || time.defined()
    ;
  }
  bool SystemTime::hasAllData() const {
    return 
         sid.defined()
      && source.defined()
      && date.defined()
      && time.defined()
    ;
  }
  bool SystemTime::valid() const {
    return true
       && source.defined()
    ;
  }
  std::vector<uint8_t> SystemTime::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode SystemTime";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushUnsigned(4, source.cast<uint64_t>());
      dst.fillBits(4, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0, date);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0, time);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  Rudder::Rudder() {
  }

  Rudder::Rudder(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (48 <= src.remainingBits()) {
      instance = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      directionOrder = src.getUnsigned(2, N2kField::Definedness::AlwaysDefined);
        // Skipping reserved
        src.advanceBits(6);
      angleOrder = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      position = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
    // No repeating fields.
    }
  }
  bool Rudder::hasSomeData() const {
    return 
         instance.defined()
      || directionOrder.defined()
      || angleOrder.defined()
      || position.defined()
    ;
  }
  bool Rudder::hasAllData() const {
    return 
         instance.defined()
      && directionOrder.defined()
      && angleOrder.defined()
      && position.defined()
    ;
  }
  bool Rudder::valid() const {
    return true
    ;
  }
  std::vector<uint8_t> Rudder::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode Rudder";
      return {};
    }
      dst.pushUnsigned(8, instance);
      dst.pushUnsigned(2, directionOrder);
      dst.fillBits(6, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, angleOrder);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, position);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  VesselHeading::VesselHeading() {
  }

  VesselHeading::VesselHeading(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (58 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      heading = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      deviation = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      variation = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      reference = src.getUnsignedInSet(2, {0, 1}).cast<Reference>();
    // No repeating fields.
    }
  }
  bool VesselHeading::hasSomeData() const {
    return 
         sid.defined()
      || heading.defined()
      || deviation.defined()
      || variation.defined()
      || reference.defined()
    ;
  }
  bool VesselHeading::hasAllData() const {
    return 
         sid.defined()
      && heading.defined()
      && deviation.defined()
      && variation.defined()
      && reference.defined()
    ;
  }
  bool VesselHeading::valid() const {
    return true
       && reference.defined()
    ;
  }
  std::vector<uint8_t> VesselHeading::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode VesselHeading";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, heading);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, deviation);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, variation);
      dst.pushUnsigned(2, reference.cast<uint64_t>());
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  RateOfTurn::RateOfTurn() {
  }

  RateOfTurn::RateOfTurn(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (40 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      rate = src.getPhysicalQuantity(true, 3.125e-08, (sail::Angle<double>::radians(1.0)/sail::Duration<double>::seconds(1.0)), 32, 0);
    // No repeating fields.
    }
  }
  bool RateOfTurn::hasSomeData() const {
    return 
         sid.defined()
      || rate.defined()
    ;
  }
  bool RateOfTurn::hasAllData() const {
    return 
         sid.defined()
      && rate.defined()
    ;
  }
  bool RateOfTurn::valid() const {
    return true
    ;
  }
  std::vector<uint8_t> RateOfTurn::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode RateOfTurn";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(true, 3.125e-08, (sail::Angle<double>::radians(1.0)/sail::Duration<double>::seconds(1.0)), 32, 0, rate);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  Attitude::Attitude() {
  }

  Attitude::Attitude(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (56 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      yaw = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      pitch = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      roll = src.getPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
    // No repeating fields.
    }
  }
  bool Attitude::hasSomeData() const {
    return 
         sid.defined()
      || yaw.defined()
      || pitch.defined()
      || roll.defined()
    ;
  }
  bool Attitude::hasAllData() const {
    return 
         sid.defined()
      && yaw.defined()
      && pitch.defined()
      && roll.defined()
    ;
  }
  bool Attitude::valid() const {
    return true
    ;
  }
  std::vector<uint8_t> Attitude::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode Attitude";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, yaw);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, pitch);
      dst.pushPhysicalQuantity(true, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, roll);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  EngineParametersRapidUpdate::EngineParametersRapidUpdate() {
  }

  EngineParametersRapidUpdate::EngineParametersRapidUpdate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (48 <= src.remainingBits()) {
      engineInstance = src.getUnsignedInSet(8, {0, 1}).cast<EngineInstance>();
      engineSpeed = src.getPhysicalQuantity(false, 0.25, (sail::Angle<double>::degrees(360)/sail::Duration<double>::minutes(1.0)), 16, 0);
      engineBoostPressure = src.getUnsigned(16, N2kField::Definedness::MaybeUndefined);
      engineTiltTrim = src.getSigned(8, 0, N2kField::Definedness::AlwaysDefined);
    // No repeating fields.
    }
  }
  bool EngineParametersRapidUpdate::hasSomeData() const {
    return 
         engineInstance.defined()
      || engineSpeed.defined()
      || engineBoostPressure.defined()
      || engineTiltTrim.defined()
    ;
  }
  bool EngineParametersRapidUpdate::hasAllData() const {
    return 
         engineInstance.defined()
      && engineSpeed.defined()
      && engineBoostPressure.defined()
      && engineTiltTrim.defined()
    ;
  }
  bool EngineParametersRapidUpdate::valid() const {
    return true
       && engineInstance.defined()
    ;
  }
  std::vector<uint8_t> EngineParametersRapidUpdate::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode EngineParametersRapidUpdate";
      return {};
    }
      dst.pushUnsigned(8, engineInstance.cast<uint64_t>());
      dst.pushPhysicalQuantity(false, 0.25, (sail::Angle<double>::degrees(360)/sail::Duration<double>::minutes(1.0)), 16, 0, engineSpeed);
      dst.pushUnsigned(16, engineBoostPressure);
      dst.pushSigned(8, 0, engineTiltTrim);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  Speed::Speed() {
  }

  Speed::Speed(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (52 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      speedWaterReferenced = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      speedGroundReferenced = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      speedWaterReferencedType = src.getUnsignedInSet(8, {0, 1, 2, 3, 4}).cast<SpeedWaterReferencedType>();
      speedDirection = src.getUnsigned(4, N2kField::Definedness::AlwaysDefined);
    // No repeating fields.
    }
  }
  bool Speed::hasSomeData() const {
    return 
         sid.defined()
      || speedWaterReferenced.defined()
      || speedGroundReferenced.defined()
      || speedWaterReferencedType.defined()
      || speedDirection.defined()
    ;
  }
  bool Speed::hasAllData() const {
    return 
         sid.defined()
      && speedWaterReferenced.defined()
      && speedGroundReferenced.defined()
      && speedWaterReferencedType.defined()
      && speedDirection.defined()
    ;
  }
  bool Speed::valid() const {
    return true
       && speedWaterReferencedType.defined()
    ;
  }
  std::vector<uint8_t> Speed::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode Speed";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, speedWaterReferenced);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, speedGroundReferenced);
      dst.pushUnsigned(8, speedWaterReferencedType.cast<uint64_t>());
      dst.pushUnsigned(4, speedDirection);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  PositionRapidUpdate::PositionRapidUpdate() {
  }

  PositionRapidUpdate::PositionRapidUpdate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      latitude = src.getPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0);
      longitude = src.getPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0);
    // No repeating fields.
    }
  }
  bool PositionRapidUpdate::hasSomeData() const {
    return 
         latitude.defined()
      || longitude.defined()
    ;
  }
  bool PositionRapidUpdate::hasAllData() const {
    return 
         latitude.defined()
      && longitude.defined()
    ;
  }
  bool PositionRapidUpdate::valid() const {
    return true
    ;
  }
  std::vector<uint8_t> PositionRapidUpdate::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode PositionRapidUpdate";
      return {};
    }
      dst.pushPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0, latitude);
      dst.pushPhysicalQuantity(true, 0.0000001, sail::Angle<double>::degrees(1.0), 32, 0, longitude);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  CogSogRapidUpdate::CogSogRapidUpdate() {
  }

  CogSogRapidUpdate::CogSogRapidUpdate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      cogReference = src.getUnsignedInSet(2, {0, 1}).cast<CogReference>();
        // Skipping reserved
        src.advanceBits(6);
      cog = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      sog = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
        // Skipping reserved
        src.advanceBits(16);
    // No repeating fields.
    }
  }
  bool CogSogRapidUpdate::hasSomeData() const {
    return 
         sid.defined()
      || cogReference.defined()
      || cog.defined()
      || sog.defined()
    ;
  }
  bool CogSogRapidUpdate::hasAllData() const {
    return 
         sid.defined()
      && cogReference.defined()
      && cog.defined()
      && sog.defined()
    ;
  }
  bool CogSogRapidUpdate::valid() const {
    return true
       && cogReference.defined()
    ;
  }
  std::vector<uint8_t> CogSogRapidUpdate::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode CogSogRapidUpdate";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushUnsigned(2, cogReference.cast<uint64_t>());
      dst.fillBits(6, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, cog);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, sog);
      dst.fillBits(16, true); // TODO: Can we safely do this? The field name is 'Reserved'
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  GnssPositionData::GnssPositionData() {
  }

  GnssPositionData::GnssPositionData(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (344 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      date = src.getPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0);
      time = src.getPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0);
      latitude = src.getPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0);
      longitude = src.getPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0);
      altitude = src.getPhysicalQuantity(true, 1e-06, sail::Length<double>::meters(1.0), 64, 0);
      gnssType = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5, 6, 7, 8}).cast<GnssType>();
      method = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5, 6, 7, 8}).cast<Method>();
      integrity = src.getUnsignedInSet(2, {0, 1, 2}).cast<Integrity>();
        // Skipping reserved
        src.advanceBits(6);
      numberOfSvs = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      hdop = src.getDoubleWithResolution(0.01, true, 16, 0, N2kField::Definedness::MaybeUndefined);
      pdop = src.getDoubleWithResolution(0.01, true, 16, 0, N2kField::Definedness::MaybeUndefined);
      geoidalSeparation = src.getPhysicalQuantity(true, 0.01, sail::Length<double>::meters(1.0), 32, 0);
      referenceStations = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      while (32 <= src.remainingBits()) {
        auto l_referenceStationType = src.getUnsignedInSet(4, {0, 1, 2, 3, 4, 5, 6, 7, 8}).cast<ReferenceStationType>();
        auto l_referenceStationId = src.getUnsigned(12, N2kField::Definedness::MaybeUndefined);
        auto l_ageOfDgnssCorrections = src.getPhysicalQuantity(false, 0.01, sail::Duration<double>::seconds(1.0), 16, 0);
        repeating.push_back({l_referenceStationType, l_referenceStationId, l_ageOfDgnssCorrections});
      }
    }
  }
  bool GnssPositionData::hasSomeData() const {
    return 
         sid.defined()
      || date.defined()
      || time.defined()
      || latitude.defined()
      || longitude.defined()
      || altitude.defined()
      || gnssType.defined()
      || method.defined()
      || integrity.defined()
      || numberOfSvs.defined()
      || hdop.defined()
      || pdop.defined()
      || geoidalSeparation.defined()
      || referenceStations.defined()
    ;
  }
  bool GnssPositionData::hasAllData() const {
    return 
         sid.defined()
      && date.defined()
      && time.defined()
      && latitude.defined()
      && longitude.defined()
      && altitude.defined()
      && gnssType.defined()
      && method.defined()
      && integrity.defined()
      && numberOfSvs.defined()
      && hdop.defined()
      && pdop.defined()
      && geoidalSeparation.defined()
      && referenceStations.defined()
    ;
  }
  bool GnssPositionData::valid() const {
    return true
       && gnssType.defined()
       && method.defined()
       && integrity.defined()
    ;
  }
  std::vector<uint8_t> GnssPositionData::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode GnssPositionData";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0, date);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0, time);
      dst.pushPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0, latitude);
      dst.pushPhysicalQuantity(true, 0.0000000000000001, sail::Angle<double>::degrees(1.0), 64, 0, longitude);
      dst.pushPhysicalQuantity(true, 1e-06, sail::Length<double>::meters(1.0), 64, 0, altitude);
      dst.pushUnsigned(4, gnssType.cast<uint64_t>());
      dst.pushUnsigned(4, method.cast<uint64_t>());
      dst.pushUnsigned(2, integrity.cast<uint64_t>());
      dst.fillBits(6, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushUnsigned(8, numberOfSvs);
      dst.pushDoubleWithResolution(0.01, true, 16, 0, hdop);
      dst.pushDoubleWithResolution(0.01, true, 16, 0, pdop);
      dst.pushPhysicalQuantity(true, 0.01, sail::Length<double>::meters(1.0), 32, 0, geoidalSeparation);
      dst.pushUnsigned(8, referenceStations);
    for (const auto& x: repeating) {
      dst.pushUnsigned(4, x.referenceStationType.cast<uint64_t>());
      dst.pushUnsigned(12, x.referenceStationId);
      dst.pushPhysicalQuantity(false, 0.01, sail::Duration<double>::seconds(1.0), 16, 0, x.ageOfDgnssCorrections);
    }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  TimeDate::TimeDate() {
  }

  TimeDate::TimeDate(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      date = src.getPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0);
      time = src.getPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0);
      localOffset = src.getPhysicalQuantity(true, 1, sail::Duration<double>::minutes(1.0), 16, 0);
    // No repeating fields.
    }
  }
  bool TimeDate::hasSomeData() const {
    return 
         date.defined()
      || time.defined()
      || localOffset.defined()
    ;
  }
  bool TimeDate::hasAllData() const {
    return 
         date.defined()
      && time.defined()
      && localOffset.defined()
    ;
  }
  bool TimeDate::valid() const {
    return true
    ;
  }
  std::vector<uint8_t> TimeDate::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode TimeDate";
      return {};
    }
      dst.pushPhysicalQuantity(false, 1, sail::Duration<double>::days(1.0), 16, 0, date);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Duration<double>::seconds(1.0), 32, 0, time);
      dst.pushPhysicalQuantity(true, 1, sail::Duration<double>::minutes(1.0), 16, 0, localOffset);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  WindData::WindData() {
  }

  WindData::WindData(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (43 <= src.remainingBits()) {
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      windSpeed = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      windAngle = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      reference = src.getUnsignedInSet(3, {0, 1, 2, 3, 4}).cast<Reference>();
    // No repeating fields.
    }
  }
  bool WindData::hasSomeData() const {
    return 
         sid.defined()
      || windSpeed.defined()
      || windAngle.defined()
      || reference.defined()
    ;
  }
  bool WindData::hasAllData() const {
    return 
         sid.defined()
      && windSpeed.defined()
      && windAngle.defined()
      && reference.defined()
    ;
  }
  bool WindData::valid() const {
    return true
       && reference.defined()
    ;
  }
  std::vector<uint8_t> WindData::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode WindData";
      return {};
    }
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, windSpeed);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, windAngle);
      dst.pushUnsigned(3, reference.cast<uint64_t>());
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  DirectionData::DirectionData() {
  }

  DirectionData::DirectionData(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (112 <= src.remainingBits()) {
      dataMode = src.getUnsignedInSet(4, {0, 1, 2, 3, 4}).cast<DataMode>();
      cogReference = src.getUnsignedInSet(2, {0, 1}).cast<CogReference>();
        // Skipping reserved
        src.advanceBits(2);
      sid = src.getUnsigned(8, N2kField::Definedness::AlwaysDefined);
      cog = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      sog = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      heading = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      speedThroughWater = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
      set = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
      drift = src.getPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0);
    // No repeating fields.
    }
  }
  bool DirectionData::hasSomeData() const {
    return 
         dataMode.defined()
      || cogReference.defined()
      || sid.defined()
      || cog.defined()
      || sog.defined()
      || heading.defined()
      || speedThroughWater.defined()
      || set.defined()
      || drift.defined()
    ;
  }
  bool DirectionData::hasAllData() const {
    return 
         dataMode.defined()
      && cogReference.defined()
      && sid.defined()
      && cog.defined()
      && sog.defined()
      && heading.defined()
      && speedThroughWater.defined()
      && set.defined()
      && drift.defined()
    ;
  }
  bool DirectionData::valid() const {
    return true
       && dataMode.defined()
       && cogReference.defined()
    ;
  }
  std::vector<uint8_t> DirectionData::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode DirectionData";
      return {};
    }
      dst.pushUnsigned(4, dataMode.cast<uint64_t>());
      dst.pushUnsigned(2, cogReference.cast<uint64_t>());
      dst.fillBits(2, true); // TODO: Can we safely do this? The field name is 'Reserved'
      dst.pushUnsigned(8, sid);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, cog);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, sog);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, heading);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, speedThroughWater);
      dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, set);
      dst.pushPhysicalQuantity(false, 0.01, sail::Velocity<double>::metersPerSecond(1.0), 16, 0, drift);
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

  BandGVmgPerformance::BandGVmgPerformance() {
  }

  BandGVmgPerformance::BandGVmgPerformance(const uint8_t *data, int lengthBytes) {
    N2kField::N2kFieldStream src(data, lengthBytes);
    if (64 <= src.remainingBits()) {
      manufacturerId = src.getUnsigned(16, N2kField::Definedness::MaybeUndefined);
      dataId = src.getUnsignedInSet(12, {105, 285}).cast<DataId>();
      length = src.getUnsigned(4, N2kField::Definedness::AlwaysDefined);
        if ((dataId.defined() && dataId.get() == static_cast<DataId>(285))) {
          vmgPerformance = src.getDoubleWithResolution(0.001, true, 16, 0, N2kField::Definedness::MaybeUndefined);
        }
        if ((dataId.defined() && dataId.get() == static_cast<DataId>(105))) {
          course = src.getPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0);
        }
    // No repeating fields.
    }
  }
  bool BandGVmgPerformance::hasSomeData() const {
    return 
         manufacturerId.defined()
      || dataId.defined()
      || length.defined()
    ;
  }
  bool BandGVmgPerformance::hasAllData() const {
    return 
         manufacturerId.defined()
      && dataId.defined()
      && length.defined()
    ;
  }
  bool BandGVmgPerformance::valid() const {
    return true
       && manufacturerId.defined() && manufacturerId.get() == 39293
       && dataId.defined()
       && length.defined() && length.get() == 2
    ;
  }
  std::vector<uint8_t> BandGVmgPerformance::encode() const {
    N2kField::N2kFieldOutputStream dst;
    if (!valid()) {
      std::cerr << "Cannot encode BandGVmgPerformance";
      return {};
    }
      dst.pushUnsigned(16, manufacturerId);
      dst.pushUnsigned(12, dataId.cast<uint64_t>());
      dst.pushUnsigned(4, length);
        if ((dataId.defined() && dataId.get() == static_cast<DataId>(285))) {
          dst.pushDoubleWithResolution(0.001, true, 16, 0, vmgPerformance);
        }
        if ((dataId.defined() && dataId.get() == static_cast<DataId>(105))) {
          dst.pushPhysicalQuantity(false, 0.0001, sail::Angle<double>::radians(1.0), 16, 0, course);
        }
    dst.fillUpToLength(8*8, true);
    return dst.moveData();
  }

bool isFastPacket(int pgn) {
  return (pgn == 129029); // TODO: This is just temporary.
}

bool PgnVisitor::visit(const tN2kMsg &packet) {
  switch(packet.PGN) {
    case 60160: return apply(packet, IsoTransportProtocolDataTransfer(packet.Data, packet.DataLen));
    case 60416: {
      BitStream dispatchStream(packet.Data, packet.DataLen);
      auto dispatchCode0 = dispatchStream.getUnsigned(8);
      switch(dispatchCode0) {
        case 16: return apply(packet, IsoTransportProtocolConnectionManagementRequestToSend(packet.Data, packet.DataLen));
        case 17: return apply(packet, IsoTransportProtocolConnectionManagementClearToSend(packet.Data, packet.DataLen));
        case 19: return apply(packet, IsoTransportProtocolConnectionManagementEndOfMessage(packet.Data, packet.DataLen));
        case 32: return apply(packet, IsoTransportProtocolConnectionManagementBroadcastAnnounce(packet.Data, packet.DataLen));
        case 255: return apply(packet, IsoTransportProtocolConnectionManagementAbort(packet.Data, packet.DataLen));
        default: return false;
      };
      break;
    }
    case 65330: return apply(packet, BandGVmgPerformance(packet.Data, packet.DataLen));
    case 126992: return apply(packet, SystemTime(packet.Data, packet.DataLen));
    case 127245: return apply(packet, Rudder(packet.Data, packet.DataLen));
    case 127250: return apply(packet, VesselHeading(packet.Data, packet.DataLen));
    case 127251: return apply(packet, RateOfTurn(packet.Data, packet.DataLen));
    case 127257: return apply(packet, Attitude(packet.Data, packet.DataLen));
    case 127488: return apply(packet, EngineParametersRapidUpdate(packet.Data, packet.DataLen));
    case 128259: return apply(packet, Speed(packet.Data, packet.DataLen));
    case 129025: return apply(packet, PositionRapidUpdate(packet.Data, packet.DataLen));
    case 129026: return apply(packet, CogSogRapidUpdate(packet.Data, packet.DataLen));
    case 129029: return apply(packet, GnssPositionData(packet.Data, packet.DataLen));
    case 129033: return apply(packet, TimeDate(packet.Data, packet.DataLen));
    case 130306: return apply(packet, WindData(packet.Data, packet.DataLen));
    case 130577: return apply(packet, DirectionData(packet.Data, packet.DataLen));
    default: return false;
  };
  return false;
}
}