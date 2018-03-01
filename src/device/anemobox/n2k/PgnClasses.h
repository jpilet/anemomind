/** Generated on Thu Mar 01 2018 13:41:03 GMT+0100 (CET) using 
 *
 *     /usr/local/bin/node /Users/jonas/prog/anemomind/src/device/anemobox/n2k/codegen/index.js /Users/jonas/prog/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#ifndef _PGNCLASSES_HEADER_
#define _PGNCLASSES_HEADER_ 1

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <cassert>
#include <device/anemobox/n2k/N2kField.h>
#include <server/common/Optional.h>
#include <N2kMsg.h>

namespace PgnClasses {
  class PgnBaseClass {
  public:
    virtual int code() const = 0;
    virtual std::vector<uint8_t> encode() const = 0;
    virtual ~PgnBaseClass() {}
  };
  
  enum class PgnVariant60416 {
    TypeIsoTransportProtocolConnectionManagementRequestToSend,
    TypeIsoTransportProtocolConnectionManagementClearToSend,
    TypeIsoTransportProtocolConnectionManagementEndOfMessage,
    TypeIsoTransportProtocolConnectionManagementBroadcastAnnounce,
    TypeIsoTransportProtocolConnectionManagementAbort,
    Undefined
  };

  
  struct IsoTransportProtocolDataTransfer: public PgnBaseClass { // ISO Transport Protocol, Data Transfer
    // Minimum size: 8 bits = 1 bytes. Repeating struct size: 56 bits = 7 bytes
    static const int ThisPgn = 60160;
    int code() const override {return 60160;}
    struct Repeating {
      Optional<sail::Array<uint8_t> > data; //  at 8 bits = 1 bytes
    };

    IsoTransportProtocolDataTransfer();
    IsoTransportProtocolDataTransfer(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    std::vector<Repeating> repeating;
  };
  
  struct IsoTransportProtocolConnectionManagementRequestToSend: public PgnBaseClass { // ISO Transport Protocol, Connection Management - Request To Send
    // Minimum size: 40 bits = 5 bytes. Repeating struct size: 24 bits = 3 bytes
    static const int ThisPgn = 60416;
    int code() const override {return 60416;}
    struct Repeating {
      Optional<uint64_t > pgn; // PGN at 40 bits = 5 bytes
    };

    IsoTransportProtocolConnectionManagementRequestToSend();
    IsoTransportProtocolConnectionManagementRequestToSend(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > groupFunctionCode = 16; // RTS at 0 bits = 0 bytes
    Optional<uint64_t > messageSize; // bytes at 8 bits = 1 bytes
    Optional<uint64_t > packets; // packets at 24 bits = 3 bytes
    Optional<uint64_t > packetsReply; // packets sent in response to CTS at 32 bits = 4 bytes
    std::vector<Repeating> repeating;
  };
  
  struct IsoTransportProtocolConnectionManagementClearToSend: public PgnBaseClass { // ISO Transport Protocol, Connection Management - Clear To Send
    // Minimum size: 40 bits = 5 bytes. Repeating struct size: 24 bits = 3 bytes
    static const int ThisPgn = 60416;
    int code() const override {return 60416;}
    struct Repeating {
      Optional<uint64_t > pgn; // PGN at 40 bits = 5 bytes
    };

    IsoTransportProtocolConnectionManagementClearToSend();
    IsoTransportProtocolConnectionManagementClearToSend(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > groupFunctionCode = 17; // CTS at 0 bits = 0 bytes
    Optional<uint64_t > maxPackets; // packets before waiting for next CTS at 8 bits = 1 bytes
    Optional<uint64_t > nextSid; // packet at 16 bits = 2 bytes
    // Skip field 'Reserved' of length 16 at 24 bits = 3 bytes: Reserved field
    std::vector<Repeating> repeating;
  };
  
  struct IsoTransportProtocolConnectionManagementEndOfMessage: public PgnBaseClass { // ISO Transport Protocol, Connection Management - End Of Message
    // Minimum size: 40 bits = 5 bytes. Repeating struct size: 24 bits = 3 bytes
    static const int ThisPgn = 60416;
    int code() const override {return 60416;}
    struct Repeating {
      Optional<uint64_t > pgn; // PGN at 40 bits = 5 bytes
    };

    IsoTransportProtocolConnectionManagementEndOfMessage();
    IsoTransportProtocolConnectionManagementEndOfMessage(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > groupFunctionCode = 19; // EOM at 0 bits = 0 bytes
    Optional<uint64_t > totalMessageSize; // bytes at 8 bits = 1 bytes
    Optional<uint64_t > totalNumberOfPacketsReceived; // packets at 24 bits = 3 bytes
    // Skip field 'Reserved' of length 8 at 32 bits = 4 bytes: Reserved field
    std::vector<Repeating> repeating;
  };
  
  struct IsoTransportProtocolConnectionManagementBroadcastAnnounce: public PgnBaseClass { // ISO Transport Protocol, Connection Management - Broadcast Announce
    // Minimum size: 40 bits = 5 bytes. Repeating struct size: 24 bits = 3 bytes
    static const int ThisPgn = 60416;
    int code() const override {return 60416;}
    struct Repeating {
      Optional<uint64_t > pgn; // PGN at 40 bits = 5 bytes
    };

    IsoTransportProtocolConnectionManagementBroadcastAnnounce();
    IsoTransportProtocolConnectionManagementBroadcastAnnounce(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > groupFunctionCode = 32; // BAM at 0 bits = 0 bytes
    Optional<uint64_t > messageSize; // bytes at 8 bits = 1 bytes
    Optional<uint64_t > packets; // frames at 24 bits = 3 bytes
    // Skip field 'Reserved' of length 8 at 32 bits = 4 bytes: Reserved field
    std::vector<Repeating> repeating;
  };
  
  struct IsoTransportProtocolConnectionManagementAbort: public PgnBaseClass { // ISO Transport Protocol, Connection Management - Abort
    // Minimum size: 32 bits = 4 bytes. Repeating struct size: 24 bits = 3 bytes
    static const int ThisPgn = 60416;
    int code() const override {return 60416;}
    struct Repeating {
      Optional<uint64_t > pgn; // PGN at 32 bits = 4 bytes
    };

    IsoTransportProtocolConnectionManagementAbort();
    IsoTransportProtocolConnectionManagementAbort(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > groupFunctionCode = 255; // Abort at 0 bits = 0 bytes
    Optional<uint64_t > reason; //  at 8 bits = 1 bytes
    // Skip field 'Reserved' of length 16 at 16 bits = 2 bytes: Reserved field
    std::vector<Repeating> repeating;
  };
  
  struct SystemTime: public PgnBaseClass { // System Time
    // Minimum size: 64 bits = 8 bytes. 
    static const int ThisPgn = 126992;
    int code() const override {return 126992;}
    enum class Source {
      GPS = 0, 
      GLONASS = 1, 
      Radio_Station = 2, 
      Local_Cesium_clock = 3, 
      Local_Rubidium_clock = 4, 
      Local_Crystal_clock = 5
    };

    SystemTime();
    SystemTime(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    sail::TimeStamp timeStamp() const {
      return N2kField::getTimeStamp(*this);
    }
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<Source > source; //  at 8 bits = 1 bytes
    // Skip field 'Reserved' of length 4 at 12 bits = 1 bytes + 4 bits: Reserved field
    Optional<sail::Duration<double> > date; // Days since January 1, 1970 at 16 bits = 2 bytes
    Optional<sail::Duration<double> > time; // Seconds since midnight at 32 bits = 4 bytes
  };
  
  struct Rudder: public PgnBaseClass { // Rudder
    // Minimum size: 48 bits = 6 bytes. 
    static const int ThisPgn = 127245;
    int code() const override {return 127245;}

    Rudder();
    Rudder(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > instance; //  at 0 bits = 0 bytes
    Optional<uint64_t > directionOrder; //  at 8 bits = 1 bytes
    // Skip field 'Reserved' of length 6 at 10 bits = 1 bytes + 2 bits: Reserved field
    Optional<sail::Angle<double> > angleOrder; //  at 16 bits = 2 bytes
    Optional<sail::Angle<double> > position; //  at 32 bits = 4 bytes
  };
  
  struct VesselHeading: public PgnBaseClass { // Vessel Heading
    // Minimum size: 58 bits = 7 bytes + 2 bits. 
    static const int ThisPgn = 127250;
    int code() const override {return 127250;}
    enum class Reference {
      True = 0, 
      Magnetic = 1
    };

    VesselHeading();
    VesselHeading(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<sail::Angle<double> > heading; //  at 8 bits = 1 bytes
    Optional<sail::Angle<double> > deviation; //  at 24 bits = 3 bytes
    Optional<sail::Angle<double> > variation; //  at 40 bits = 5 bytes
    Optional<Reference > reference; //  at 56 bits = 7 bytes
  };
  
  struct RateOfTurn: public PgnBaseClass { // Rate of Turn
    // Minimum size: 40 bits = 5 bytes. 
    static const int ThisPgn = 127251;
    int code() const override {return 127251;}

    RateOfTurn();
    RateOfTurn(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<sail::AngularVelocity<double> > rate; //  at 8 bits = 1 bytes
  };
  
  struct Attitude: public PgnBaseClass { // Attitude
    // Minimum size: 56 bits = 7 bytes. 
    static const int ThisPgn = 127257;
    int code() const override {return 127257;}

    Attitude();
    Attitude(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<sail::Angle<double> > yaw; //  at 8 bits = 1 bytes
    Optional<sail::Angle<double> > pitch; //  at 24 bits = 3 bytes
    Optional<sail::Angle<double> > roll; //  at 40 bits = 5 bytes
  };
  
  struct EngineParametersRapidUpdate: public PgnBaseClass { // Engine Parameters, Rapid Update
    // Minimum size: 48 bits = 6 bytes. 
    static const int ThisPgn = 127488;
    int code() const override {return 127488;}
    enum class EngineInstance {
      Single_Engine_or_Dual_Engine_Port = 0, 
      Dual_Engine_Starboard = 1
    };

    EngineParametersRapidUpdate();
    EngineParametersRapidUpdate(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<EngineInstance > engineInstance; //  at 0 bits = 0 bytes
    Optional<sail::AngularVelocity<double> > engineSpeed; //  at 8 bits = 1 bytes
    Optional<uint64_t > engineBoostPressure; //  at 24 bits = 3 bytes
    Optional<int64_t > engineTiltTrim; //  at 40 bits = 5 bytes
  };
  
  struct Speed: public PgnBaseClass { // Speed
    // Minimum size: 52 bits = 6 bytes + 4 bits. 
    static const int ThisPgn = 128259;
    int code() const override {return 128259;}
    enum class SpeedWaterReferencedType {
      Paddle_wheel = 0, 
      Pitot_tube = 1, 
      Doppler = 2, 
      Correlation_ultra_sound = 3, 
      Electro_Magnetic = 4
    };

    Speed();
    Speed(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<sail::Velocity<double> > speedWaterReferenced; //  at 8 bits = 1 bytes
    Optional<sail::Velocity<double> > speedGroundReferenced; //  at 24 bits = 3 bytes
    Optional<SpeedWaterReferencedType > speedWaterReferencedType; //  at 40 bits = 5 bytes
    Optional<uint64_t > speedDirection; //  at 48 bits = 6 bytes
  };
  
  struct PositionRapidUpdate: public PgnBaseClass { // Position, Rapid Update
    // Minimum size: 64 bits = 8 bytes. 
    static const int ThisPgn = 129025;
    int code() const override {return 129025;}

    PositionRapidUpdate();
    PositionRapidUpdate(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<sail::Angle<double> > latitude; //  at 0 bits = 0 bytes
    Optional<sail::Angle<double> > longitude; //  at 32 bits = 4 bytes
  };
  
  struct CogSogRapidUpdate: public PgnBaseClass { // COG & SOG, Rapid Update
    // Minimum size: 64 bits = 8 bytes. 
    static const int ThisPgn = 129026;
    int code() const override {return 129026;}
    enum class CogReference {
      True = 0, 
      Magnetic = 1
    };

    CogSogRapidUpdate();
    CogSogRapidUpdate(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<CogReference > cogReference; //  at 8 bits = 1 bytes
    // Skip field 'Reserved' of length 6 at 10 bits = 1 bytes + 2 bits: Reserved field
    Optional<sail::Angle<double> > cog; //  at 16 bits = 2 bytes
    Optional<sail::Velocity<double> > sog; //  at 32 bits = 4 bytes
    // Skip field 'Reserved' of length 16 at 48 bits = 6 bytes: Reserved field
  };
  
  struct GnssPositionData: public PgnBaseClass { // GNSS Position Data
    // Minimum size: 344 bits = 43 bytes. Repeating struct size: 32 bits = 4 bytes
    static const int ThisPgn = 129029;
    int code() const override {return 129029;}
    enum class GnssType {
      GPS = 0, 
      GLONASS = 1, 
      GPS_GLONASS = 2, 
      GPS_SBAS_WAAS = 3, 
      GPS_SBAS_WAAS_GLONASS = 4, 
      Chayka = 5, 
      integrated = 6, 
      surveyed = 7, 
      Galileo = 8
    };
    enum class Method {
      no_GNSS = 0, 
      GNSS_fix = 1, 
      DGNSS_fix = 2, 
      Precise_GNSS = 3, 
      RTK_Fixed_Integer = 4, 
      RTK_float = 5, 
      Estimated_DR_mode = 6, 
      Manual_Input = 7, 
      Simulate_mode = 8
    };
    enum class Integrity {
      No_integrity_checking = 0, 
      Safe = 1, 
      Caution = 2
    };
    enum class ReferenceStationType {
      GPS = 0, 
      GLONASS = 1, 
      GPS_GLONASS = 2, 
      GPS_SBAS_WAAS = 3, 
      GPS_SBAS_WAAS_GLONASS = 4, 
      Chayka = 5, 
      integrated = 6, 
      surveyed = 7, 
      Galileo = 8
    };
    struct Repeating {
      Optional<ReferenceStationType > referenceStationType; //  at 344 bits = 43 bytes
      Optional<uint64_t > referenceStationId; //  at 348 bits = 43 bytes + 4 bits
      Optional<sail::Duration<double> > ageOfDgnssCorrections; //  at 360 bits = 45 bytes
    };

    GnssPositionData();
    GnssPositionData(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    sail::TimeStamp timeStamp() const {
      return N2kField::getTimeStamp(*this);
    }
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<sail::Duration<double> > date; // Days since January 1, 1970 at 8 bits = 1 bytes
    Optional<sail::Duration<double> > time; // Seconds since midnight at 24 bits = 3 bytes
    Optional<sail::Angle<double> > latitude; //  at 56 bits = 7 bytes
    Optional<sail::Angle<double> > longitude; //  at 120 bits = 15 bytes
    Optional<sail::Length<double> > altitude; // Altitude referenced to WGS-84 at 184 bits = 23 bytes
    Optional<GnssType > gnssType; //  at 248 bits = 31 bytes
    Optional<Method > method; //  at 252 bits = 31 bytes + 4 bits
    Optional<Integrity > integrity; //  at 256 bits = 32 bytes
    // Skip field 'Reserved' of length 6 at 258 bits = 32 bytes + 2 bits: Reserved field
    Optional<uint64_t > numberOfSvs; // Number of satellites used in solution at 264 bits = 33 bytes
    Optional<double > hdop; // Horizontal dilution of precision at 272 bits = 34 bytes
    Optional<double > pdop; // Probable dilution of precision at 288 bits = 36 bytes
    Optional<sail::Length<double> > geoidalSeparation; // Geoidal Separation at 304 bits = 38 bytes
    Optional<uint64_t > referenceStations; // Number of reference stations at 336 bits = 42 bytes
    std::vector<Repeating> repeating;
  };
  
  struct TimeDate: public PgnBaseClass { // Time & Date
    // Minimum size: 64 bits = 8 bytes. 
    static const int ThisPgn = 129033;
    int code() const override {return 129033;}

    TimeDate();
    TimeDate(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    sail::TimeStamp timeStamp() const {
      return N2kField::getTimeStamp(*this);
    }
    
    Optional<sail::Duration<double> > date; // Days since January 1, 1970 at 0 bits = 0 bytes
    Optional<sail::Duration<double> > time; // Seconds since midnight at 16 bits = 2 bytes
    Optional<sail::Duration<double> > localOffset; // Minutes at 48 bits = 6 bytes
  };
  
  struct WindData: public PgnBaseClass { // Wind Data
    // Minimum size: 43 bits = 5 bytes + 3 bits. 
    static const int ThisPgn = 130306;
    int code() const override {return 130306;}
    enum class Reference {
      True_ground_referenced_to_North = 0, 
      Magnetic_ground_referenced_to_Magnetic_North = 1, 
      Apparent = 2, 
      True_boat_referenced = 3, 
      True_water_referenced = 4
    };

    WindData();
    WindData(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > sid; //  at 0 bits = 0 bytes
    Optional<sail::Velocity<double> > windSpeed; //  at 8 bits = 1 bytes
    Optional<sail::Angle<double> > windAngle; //  at 24 bits = 3 bytes
    Optional<Reference > reference; //  at 40 bits = 5 bytes
  };
  
  struct DirectionData: public PgnBaseClass { // Direction Data
    // Minimum size: 112 bits = 14 bytes. 
    static const int ThisPgn = 130577;
    int code() const override {return 130577;}
    enum class DataMode {
      Autonomous = 0, 
      Differential_enhanced = 1, 
      Estimated = 2, 
      Simulator = 3, 
      Manual = 4
    };
    enum class CogReference {
      True = 0, 
      Magnetic = 1
    };

    DirectionData();
    DirectionData(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<DataMode > dataMode; //  at 0 bits = 0 bytes
    Optional<CogReference > cogReference; //  at 4 bits = 0 bytes + 4 bits
    // Skip field 'Reserved' of length 2 at 6 bits = 0 bytes + 6 bits: Reserved field
    Optional<uint64_t > sid; //  at 8 bits = 1 bytes
    Optional<sail::Angle<double> > cog; //  at 16 bits = 2 bytes
    Optional<sail::Velocity<double> > sog; //  at 32 bits = 4 bytes
    Optional<sail::Angle<double> > heading; //  at 48 bits = 6 bytes
    Optional<sail::Velocity<double> > speedThroughWater; //  at 64 bits = 8 bytes
    Optional<sail::Angle<double> > set; //  at 80 bits = 10 bytes
    Optional<sail::Velocity<double> > drift; //  at 96 bits = 12 bytes
  };
  
  struct BandGVmgPerformancePercentage: public PgnBaseClass { // B&G Properietary PGN -- VMG Performance %
    // Minimum size: 48 bits = 6 bytes. 
    static const int ThisPgn = 65330;
    int code() const override {return 65330;}
    enum class DataId {
      VMG_target_percentage = 285
    };

    BandGVmgPerformancePercentage();
    BandGVmgPerformancePercentage(const uint8_t *data, int lengthBytes);
    bool hasSomeData() const;
    bool hasAllData() const;
    bool valid() const;
    std::vector<uint8_t> encode() const override;
    
    Optional<uint64_t > manufacturerId = 39293; // B&G at 0 bits = 0 bytes
    Optional<DataId > dataId = DataId::VMG_target_percentage; //  at 16 bits = 2 bytes
    Optional<uint64_t > length = 2; // Not sure if the length is in bytes or bits. at 28 bits = 3 bytes + 4 bits
    Optional<double > value; // 'Each bit is 0.1%' according to NDA. at 32 bits = 4 bytes
  };
  


  class PgnVisitor {
   public:
    bool visit(const tN2kMsg& packet);
    
    // You may have to split the packet, based on the pgn.
    virtual ~PgnVisitor() {}
   protected:
    virtual bool apply(const tN2kMsg& src, const IsoTransportProtocolDataTransfer& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const IsoTransportProtocolConnectionManagementRequestToSend& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const IsoTransportProtocolConnectionManagementClearToSend& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const IsoTransportProtocolConnectionManagementEndOfMessage& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const IsoTransportProtocolConnectionManagementBroadcastAnnounce& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const IsoTransportProtocolConnectionManagementAbort& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const SystemTime& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const Rudder& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const VesselHeading& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const RateOfTurn& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const Attitude& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const EngineParametersRapidUpdate& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const Speed& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const PositionRapidUpdate& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const CogSogRapidUpdate& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const GnssPositionData& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const TimeDate& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const WindData& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const DirectionData& packet) { return false; }
    virtual bool apply(const tN2kMsg& src, const BandGVmgPerformancePercentage& packet) { return false; }
  };
}

#endif