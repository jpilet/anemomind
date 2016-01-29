/** Generated on Fri Jan 29 2016 15:11:10 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#ifndef _PGNCLASSES_HEADER_
#define _PGNCLASSES_HEADER_ 1

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <cassert>
#include <device/anemobox/n2k/N2kField.h>
#include <server/common/Optional.h>

namespace PgnClasses {
  enum class PgnVariant60416 {
    TypeIsoTransportProtocolConnectionManagementRequestToSend,
    TypeIsoTransportProtocolConnectionManagementClearToSend,
    TypeIsoTransportProtocolConnectionManagementEndOfMessage,
    TypeIsoTransportProtocolConnectionManagementBroadcastAnnounce,
    TypeIsoTransportProtocolConnectionManagementAbort,
    Undefined
  };


  enum class PgnVariant130850 {
    TypeSimnetEventCommandApCommand,
    TypeSimnetEventCommandAlarm,
    TypeSimnetEventCommandUnknown,
    Undefined
  };


  // ISO Transport Protocol, Data Transfer
  class IsoTransportProtocolDataTransfer {
  public:
    static const int ThisPgn = 60160;

    IsoTransportProtocolDataTransfer();
    IsoTransportProtocolDataTransfer(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<sail::Array<uint8_t> > &data() const {return _data;}
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<sail::Array<uint8_t> > _data; 
  };

  // ISO Transport Protocol, Connection Management - Request To Send
  class IsoTransportProtocolConnectionManagementRequestToSend {
  public:
    static const int ThisPgn = 60416;

    IsoTransportProtocolConnectionManagementRequestToSend();
    IsoTransportProtocolConnectionManagementRequestToSend(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &groupFunctionCode() const {return _groupFunctionCode;}
    const Optional<uint64_t > &messageSize() const {return _messageSize;}
    const Optional<uint64_t > &packets() const {return _packets;}
    const Optional<uint64_t > &packetsReply() const {return _packetsReply;}
    const Optional<uint64_t > &pgn() const {return _pgn;}
  private:
    bool _valid;
    Optional<uint64_t > _groupFunctionCode; // RTS
    Optional<uint64_t > _messageSize; // bytes
    Optional<uint64_t > _packets; // packets
    Optional<uint64_t > _packetsReply; // packets sent in response to CTS
    Optional<uint64_t > _pgn; // PGN
  };

  // ISO Transport Protocol, Connection Management - Clear To Send
  class IsoTransportProtocolConnectionManagementClearToSend {
  public:
    static const int ThisPgn = 60416;

    IsoTransportProtocolConnectionManagementClearToSend();
    IsoTransportProtocolConnectionManagementClearToSend(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &groupFunctionCode() const {return _groupFunctionCode;}
    const Optional<uint64_t > &maxPackets() const {return _maxPackets;}
    const Optional<uint64_t > &nextSid() const {return _nextSid;}
    const Optional<uint64_t > &pgn() const {return _pgn;}
  private:
    bool _valid;
    Optional<uint64_t > _groupFunctionCode; // CTS
    Optional<uint64_t > _maxPackets; // packets before waiting for next CTS
    Optional<uint64_t > _nextSid; // packet
    Optional<uint64_t > _pgn; // PGN
  };

  // ISO Transport Protocol, Connection Management - End Of Message
  class IsoTransportProtocolConnectionManagementEndOfMessage {
  public:
    static const int ThisPgn = 60416;

    IsoTransportProtocolConnectionManagementEndOfMessage();
    IsoTransportProtocolConnectionManagementEndOfMessage(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &groupFunctionCode() const {return _groupFunctionCode;}
    const Optional<uint64_t > &totalMessageSize() const {return _totalMessageSize;}
    const Optional<uint64_t > &totalNumberOfPacketsReceived() const {return _totalNumberOfPacketsReceived;}
    const Optional<uint64_t > &pgn() const {return _pgn;}
  private:
    bool _valid;
    Optional<uint64_t > _groupFunctionCode; // EOM
    Optional<uint64_t > _totalMessageSize; // bytes
    Optional<uint64_t > _totalNumberOfPacketsReceived; // packets
    Optional<uint64_t > _pgn; // PGN
  };

  // ISO Transport Protocol, Connection Management - Broadcast Announce
  class IsoTransportProtocolConnectionManagementBroadcastAnnounce {
  public:
    static const int ThisPgn = 60416;

    IsoTransportProtocolConnectionManagementBroadcastAnnounce();
    IsoTransportProtocolConnectionManagementBroadcastAnnounce(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &groupFunctionCode() const {return _groupFunctionCode;}
    const Optional<uint64_t > &messageSize() const {return _messageSize;}
    const Optional<uint64_t > &packets() const {return _packets;}
    const Optional<uint64_t > &pgn() const {return _pgn;}
  private:
    bool _valid;
    Optional<uint64_t > _groupFunctionCode; // BAM
    Optional<uint64_t > _messageSize; // bytes
    Optional<uint64_t > _packets; // frames
    Optional<uint64_t > _pgn; // PGN
  };

  // ISO Transport Protocol, Connection Management - Abort
  class IsoTransportProtocolConnectionManagementAbort {
  public:
    static const int ThisPgn = 60416;

    IsoTransportProtocolConnectionManagementAbort();
    IsoTransportProtocolConnectionManagementAbort(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &groupFunctionCode() const {return _groupFunctionCode;}
    const Optional<uint64_t > &reason() const {return _reason;}
    const Optional<uint64_t > &pgn() const {return _pgn;}
  private:
    bool _valid;
    Optional<uint64_t > _groupFunctionCode; // Abort
    Optional<uint64_t > _reason; 
    Optional<uint64_t > _pgn; // PGN
  };

  // System Time
  class SystemTime {
  public:
    static const int ThisPgn = 126992;
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
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<Source > &source() const {return _source;}
    const Optional<sail::Duration<double> > &date() const {return _date;}
    const Optional<sail::Duration<double> > &time() const {return _time;}
    sail::TimeStamp timeStamp() const {
      return N2kField::getTimeStamp(*this);
    }
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<Source > _source; 
    Optional<sail::Duration<double> > _date; // Days since January 1, 1970
    Optional<sail::Duration<double> > _time; // Seconds since midnight
  };

  // Vessel Heading
  class VesselHeading {
  public:
    static const int ThisPgn = 127250;
    enum class Reference {
      True = 0, 
      Magnetic = 1
    };

    VesselHeading();
    VesselHeading(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<sail::Angle<double> > &heading() const {return _heading;}
    const Optional<sail::Angle<double> > &deviation() const {return _deviation;}
    const Optional<sail::Angle<double> > &variation() const {return _variation;}
    const Optional<Reference > &reference() const {return _reference;}
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<sail::Angle<double> > _heading; 
    Optional<sail::Angle<double> > _deviation; 
    Optional<sail::Angle<double> > _variation; 
    Optional<Reference > _reference; 
  };

  // Attitude
  class Attitude {
  public:
    static const int ThisPgn = 127257;

    Attitude();
    Attitude(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
  };

  // Speed
  class Speed {
  public:
    static const int ThisPgn = 128259;
    enum class SpeedWaterReferencedType {
      Paddle_wheel = 0, 
      Pitot_tube = 1, 
      Doppler = 2, 
      Correlation_ultra_sound = 3, 
      Electro_Magnetic = 4
    };

    Speed();
    Speed(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<sail::Velocity<double> > &speedWaterReferenced() const {return _speedWaterReferenced;}
    const Optional<sail::Velocity<double> > &speedGroundReferenced() const {return _speedGroundReferenced;}
    const Optional<SpeedWaterReferencedType > &speedWaterReferencedType() const {return _speedWaterReferencedType;}
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<sail::Velocity<double> > _speedWaterReferenced; 
    Optional<sail::Velocity<double> > _speedGroundReferenced; 
    Optional<SpeedWaterReferencedType > _speedWaterReferencedType; 
  };

  // Position, Rapid Update
  class PositionRapidUpdate {
  public:
    static const int ThisPgn = 129025;

    PositionRapidUpdate();
    PositionRapidUpdate(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<sail::Angle<double> > &latitude() const {return _latitude;}
    const Optional<sail::Angle<double> > &longitude() const {return _longitude;}
  private:
    bool _valid;
    Optional<sail::Angle<double> > _latitude; 
    Optional<sail::Angle<double> > _longitude; 
  };

  // COG & SOG, Rapid Update
  class CogSogRapidUpdate {
  public:
    static const int ThisPgn = 129026;
    enum class CogReference {
      True = 0, 
      Magnetic = 1
    };

    CogSogRapidUpdate();
    CogSogRapidUpdate(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<CogReference > &cogReference() const {return _cogReference;}
    const Optional<sail::Angle<double> > &cog() const {return _cog;}
    const Optional<sail::Velocity<double> > &sog() const {return _sog;}
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<CogReference > _cogReference; 
    Optional<sail::Angle<double> > _cog; 
    Optional<sail::Velocity<double> > _sog; 
  };

  // GNSS Position Data
  class GnssPositionData {
  public:
    static const int ThisPgn = 129029;
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

    GnssPositionData();
    GnssPositionData(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<sail::Duration<double> > &date() const {return _date;}
    const Optional<sail::Duration<double> > &time() const {return _time;}
    const Optional<sail::Angle<double> > &latitude() const {return _latitude;}
    const Optional<sail::Angle<double> > &longitude() const {return _longitude;}
    const Optional<sail::Length<double> > &altitude() const {return _altitude;}
    const Optional<GnssType > &gnssType() const {return _gnssType;}
    const Optional<Method > &method() const {return _method;}
    const Optional<Integrity > &integrity() const {return _integrity;}
    const Optional<uint64_t > &numberOfSvs() const {return _numberOfSvs;}
    const Optional<int64_t > &hdop() const {return _hdop;}
    const Optional<int64_t > &pdop() const {return _pdop;}
    const Optional<sail::Length<double> > &geoidalSeparation() const {return _geoidalSeparation;}
    const Optional<uint64_t > &referenceStations() const {return _referenceStations;}
    const Optional<ReferenceStationType > &referenceStationType() const {return _referenceStationType;}
    const Optional<sail::Duration<double> > &ageOfDgnssCorrections() const {return _ageOfDgnssCorrections;}
    sail::TimeStamp timeStamp() const {
      return N2kField::getTimeStamp(*this);
    }
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<sail::Duration<double> > _date; // Days since January 1, 1970
    Optional<sail::Duration<double> > _time; // Seconds since midnight
    Optional<sail::Angle<double> > _latitude; 
    Optional<sail::Angle<double> > _longitude; 
    Optional<sail::Length<double> > _altitude; 
    Optional<GnssType > _gnssType; 
    Optional<Method > _method; 
    Optional<Integrity > _integrity; 
    Optional<uint64_t > _numberOfSvs; // Number of satellites used in solution
    Optional<int64_t > _hdop; // Horizontal dilution of precision
    Optional<int64_t > _pdop; // Probable dilution of precision
    Optional<sail::Length<double> > _geoidalSeparation; // Geoidal Separation
    Optional<uint64_t > _referenceStations; // Number of reference stations
    Optional<ReferenceStationType > _referenceStationType; 
    Optional<sail::Duration<double> > _ageOfDgnssCorrections; 
  };

  // Time & Date
  class TimeDate {
  public:
    static const int ThisPgn = 129033;

    TimeDate();
    TimeDate(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<sail::Duration<double> > &date() const {return _date;}
    const Optional<sail::Duration<double> > &time() const {return _time;}
    const Optional<sail::Duration<double> > &localOffset() const {return _localOffset;}
    sail::TimeStamp timeStamp() const {
      return N2kField::getTimeStamp(*this);
    }
  private:
    bool _valid;
    Optional<sail::Duration<double> > _date; // Days since January 1, 1970
    Optional<sail::Duration<double> > _time; // Seconds since midnight
    Optional<sail::Duration<double> > _localOffset; // Minutes
  };

  // Wind Data
  class WindData {
  public:
    static const int ThisPgn = 130306;
    enum class Reference {
      True_ground_referenced_to_North = 0, 
      Magnetic_ground_referenced_to_Magnetic_North = 1, 
      Apparent = 2, 
      True_boat_referenced = 3, 
      True_water_referenced = 4
    };

    WindData();
    WindData(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<sail::Velocity<double> > &windSpeed() const {return _windSpeed;}
    const Optional<sail::Angle<double> > &windAngle() const {return _windAngle;}
    const Optional<Reference > &reference() const {return _reference;}
  private:
    bool _valid;
    Optional<uint64_t > _sid; 
    Optional<sail::Velocity<double> > _windSpeed; 
    Optional<sail::Angle<double> > _windAngle; 
    Optional<Reference > _reference; 
  };

  // Direction Data
  class DirectionData {
  public:
    static const int ThisPgn = 130577;
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
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<DataMode > &dataMode() const {return _dataMode;}
    const Optional<CogReference > &cogReference() const {return _cogReference;}
    const Optional<uint64_t > &sid() const {return _sid;}
    const Optional<sail::Angle<double> > &cog() const {return _cog;}
    const Optional<sail::Velocity<double> > &sog() const {return _sog;}
    const Optional<sail::Angle<double> > &heading() const {return _heading;}
    const Optional<sail::Velocity<double> > &speedThroughWater() const {return _speedThroughWater;}
    const Optional<sail::Angle<double> > &set() const {return _set;}
    const Optional<sail::Velocity<double> > &drift() const {return _drift;}
  private:
    bool _valid;
    Optional<DataMode > _dataMode; 
    Optional<CogReference > _cogReference; 
    Optional<uint64_t > _sid; 
    Optional<sail::Angle<double> > _cog; 
    Optional<sail::Velocity<double> > _sog; 
    Optional<sail::Angle<double> > _heading; 
    Optional<sail::Velocity<double> > _speedThroughWater; 
    Optional<sail::Angle<double> > _set; 
    Optional<sail::Velocity<double> > _drift; 
  };

  // Simnet: Event Command: AP command
  class SimnetEventCommandApCommand {
  public:
    static const int ThisPgn = 130850;
    enum class Event {
      Standby = 6, 
      Auto_mode = 9, 
      Nav_mode = 10, 
      Non_Follow_Up_mode = 13, 
      Wind_mode = 15, 
      Change_Course = 26
    };
    enum class Direction {
      Port = 2, 
      Starboard = 3, 
      Left_rudder_port = 4, 
      Right_rudder_starboard = 5
    };

    SimnetEventCommandApCommand();
    SimnetEventCommandApCommand(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &manufacturerCode() const {return _manufacturerCode;}
    const Optional<uint64_t > &industryCode() const {return _industryCode;}
    const Optional<uint64_t > &proprietaryId() const {return _proprietaryId;}
    const Optional<uint64_t > &b() const {return _b;}
    const Optional<uint64_t > &controllingDevice() const {return _controllingDevice;}
    const Optional<Event > &event() const {return _event;}
    const Optional<Direction > &direction() const {return _direction;}
    const Optional<sail::Angle<double> > &angle() const {return _angle;}
    const Optional<uint64_t > &g() const {return _g;}
  private:
    bool _valid;
    Optional<uint64_t > _manufacturerCode; // Simrad
    Optional<uint64_t > _industryCode; // Marine Industry
    Optional<uint64_t > _proprietaryId; // AP command
    Optional<uint64_t > _b; 
    Optional<uint64_t > _controllingDevice; 
    Optional<Event > _event; 
    Optional<Direction > _direction; 
    Optional<sail::Angle<double> > _angle; 
    Optional<uint64_t > _g; 
  };

  // Simnet: Event Command: Alarm?
  class SimnetEventCommandAlarm {
  public:
    static const int ThisPgn = 130850;
    enum class Alarm {
      Raise = 57, 
      Clear = 56
    };

    SimnetEventCommandAlarm();
    SimnetEventCommandAlarm(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &manufacturerCode() const {return _manufacturerCode;}
    const Optional<uint64_t > &industryCode() const {return _industryCode;}
    const Optional<uint64_t > &a() const {return _a;}
    const Optional<uint64_t > &proprietaryId() const {return _proprietaryId;}
    const Optional<uint64_t > &c() const {return _c;}
    const Optional<Alarm > &alarm() const {return _alarm;}
    const Optional<uint64_t > &messageId() const {return _messageId;}
    const Optional<uint64_t > &f() const {return _f;}
    const Optional<uint64_t > &g() const {return _g;}
  private:
    bool _valid;
    Optional<uint64_t > _manufacturerCode; // Simrad
    Optional<uint64_t > _industryCode; // Marine Industry
    Optional<uint64_t > _a; 
    Optional<uint64_t > _proprietaryId; // Alarm command
    Optional<uint64_t > _c; 
    Optional<Alarm > _alarm; 
    Optional<uint64_t > _messageId; 
    Optional<uint64_t > _f; 
    Optional<uint64_t > _g; 
  };

  // Simnet: Event Command: Unknown
  class SimnetEventCommandUnknown {
  public:
    static const int ThisPgn = 130850;

    SimnetEventCommandUnknown();
    SimnetEventCommandUnknown(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &manufacturerCode() const {return _manufacturerCode;}
    const Optional<uint64_t > &industryCode() const {return _industryCode;}
    const Optional<uint64_t > &a() const {return _a;}
    const Optional<uint64_t > &proprietaryId() const {return _proprietaryId;}
    const Optional<uint64_t > &b() const {return _b;}
    const Optional<uint64_t > &c() const {return _c;}
    const Optional<uint64_t > &d() const {return _d;}
    const Optional<uint64_t > &e() const {return _e;}
  private:
    bool _valid;
    Optional<uint64_t > _manufacturerCode; // Simrad
    Optional<uint64_t > _industryCode; // Marine Industry
    Optional<uint64_t > _a; 
    Optional<uint64_t > _proprietaryId; // Alarm command
    Optional<uint64_t > _b; 
    Optional<uint64_t > _c; 
    Optional<uint64_t > _d; 
    Optional<uint64_t > _e; 
  };
  


  class PgnVisitor {
   public:
    bool visit(const std::string& src, int pgn, const uint8_t *data, int length);
    virtual ~PgnVisitor() {}
    typedef std::string Context; // TODO: Extra data needed by the apply methods. Currently only a string
   protected:
    virtual bool apply(const Context& src, const IsoTransportProtocolDataTransfer& packet) { return false; }
    virtual bool apply(const Context& src, const IsoTransportProtocolConnectionManagementRequestToSend& packet) { return false; }
    virtual bool apply(const Context& src, const IsoTransportProtocolConnectionManagementClearToSend& packet) { return false; }
    virtual bool apply(const Context& src, const IsoTransportProtocolConnectionManagementEndOfMessage& packet) { return false; }
    virtual bool apply(const Context& src, const IsoTransportProtocolConnectionManagementBroadcastAnnounce& packet) { return false; }
    virtual bool apply(const Context& src, const IsoTransportProtocolConnectionManagementAbort& packet) { return false; }
    virtual bool apply(const Context& src, const SystemTime& packet) { return false; }
    virtual bool apply(const Context& src, const VesselHeading& packet) { return false; }
    virtual bool apply(const Context& src, const Attitude& packet) { return false; }
    virtual bool apply(const Context& src, const Speed& packet) { return false; }
    virtual bool apply(const Context& src, const PositionRapidUpdate& packet) { return false; }
    virtual bool apply(const Context& src, const CogSogRapidUpdate& packet) { return false; }
    virtual bool apply(const Context& src, const GnssPositionData& packet) { return false; }
    virtual bool apply(const Context& src, const TimeDate& packet) { return false; }
    virtual bool apply(const Context& src, const WindData& packet) { return false; }
    virtual bool apply(const Context& src, const DirectionData& packet) { return false; }
    virtual bool apply(const Context& src, const SimnetEventCommandApCommand& packet) { return false; }
    virtual bool apply(const Context& src, const SimnetEventCommandAlarm& packet) { return false; }
    virtual bool apply(const Context& src, const SimnetEventCommandUnknown& packet) { return false; }
    virtual PgnVariant130850 getDispatchCodeFor130850_1857(const uint8_t *data, int length) {
      return PgnVariant130850::Undefined; // TODO in derived class
    }
  };
}

#endif