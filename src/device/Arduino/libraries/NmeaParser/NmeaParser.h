  /*
 * A light NMEA library. Julien Pilet, 2008-2013.
 *
 * This library not only parses GPS sentences, but also other common marine
 * NMEA sentences such as MVW or MLW.
 *
 * This code also runs on Arduino.
 */
#ifndef NMEAPARSER_H
#define NMEAPARSER_H

#ifdef ON_SERVER
#include <string>
#include <server/common/TimeStamp.h>
#endif

#include "../PhysicalQuantity/PhysicalQuantity.h"
#include "../FixedPoint/FixedPoint.h"
#include <server/common/Optional.h>

#include <stdint.h>

typedef int32_t DWord;
typedef int16_t Word;
typedef unsigned char Byte;

#define NP_MAX_DATA_LEN 100
#define NP_MAX_ARGS 20

// Represents an accurate angle in fixed point.
// The angle is stored in degrees, minutes and 1/1000 minutes fraction.
class AccAngle {
 public:

  AccAngle();
  AccAngle(double angle) {
    _angle = angle;
  }

  // Returns the angle in degrees.
  double toDouble() const { return _angle; }

  void set(double a) { _angle = a; }

  void set(Word deg, Word min, Word mc) {
    _angle = double(deg) + ((double)min + (double)mc/1000.0) / 60.0;
  }

  void set(Word deg, double min) {
    _angle = double(deg) + min / 60.0;
  }

  void flip();

  Word deg() const;
  Word min() const;
  Word mc() const;

  AccAngle &operator = (double a) {
    set(a);
    return *this;
  }

 private:
  double _angle;
};

class GeoPos {
 public:
  GeoPos() { }
  GeoPos(double lat, double lon) : lon(lon), lat(lat) { }
  GeoPos(int latDeg, int latMin, int latMc, int lonDeg, int lonMin, int lonMc) {
    lat.set(latDeg, latMin, latMc);
    lon.set(lonDeg, lonMin, lonMc);
  }

  AccAngle lon, lat;
};

class GeoRef;
class ProjectedPos {
 public:
  ProjectedPos() {
    xy_[0] = 0;
    xy_[1] = 0;
  }
  ProjectedPos(const GeoRef &ref, const GeoPos &pos);

  double dist(const ProjectedPos &p) const;

  double x() const {
    return xy_[0];
  }
  double y() const {
    return xy_[1];
  }

 private:
  double xy_[2];
};

class GeoRef {
 public:
  GeoRef(const GeoPos &pos, double altitude) {
    set(pos, altitude);
  }
  void set(const GeoPos &pos, double altitude);

  void project(const GeoPos &pos, double xy[2]) const;
 private:
  double reflon, reflat;
  double dlon, dlat;
};

class NmeaParser {
 public:

  enum NmeaSentence {
    NMEA_NONE=0,
    NMEA_UNKNOWN,
    NMEA_RMC,
    NMEA_AW,
    NMEA_TW,
    NMEA_WAT_SP_HDG,
    NMEA_VLW,
    NMEA_GLL,
    NMEA_VTG,
    NMEA_ZDA,
    NMEA_RUDDER,
    NMEA_MWD,
    NMEA_RSA,
    NMEA_PITCH,
    NMEA_ROLL,
    NMEA_HDM,
    // Aliases must come last, otherwise we might end up with
    // multiple NMEA_XXX having the same number.
    NMEA_TIME_POS = NMEA_RMC
  };

  NmeaParser();
  NmeaSentence processByte(Byte data);
  void printSentence();
  void putSentence(void (*_putc)(char));
#ifdef ON_SERVER
  std::string sentence() const;
#endif

  DWord cumulativeWaterDistance() const {
    return cwd_;
  }
  DWord waterDistance() const {
    return wd_;
  }

  const GeoPos &pos() const {
    return pos_;
  }

  void setXTE(float dist, bool left);

  static const short INVALID_DATA_SHORT = 0x8000;
  static const char INVALID_DATA_CHAR = 0x80;
  static const long INVALID_DATA_LONG = 0x800000;

  static bool isCycleMark(NmeaSentence x) {
    return x == NMEA_RMC || x == NMEA_ZDA;
  }

  // GPS data
  sail::Velocity<FP8_8> gpsSpeed() const {
    return sail::Velocity<FP8_8>::knots(
        // 1/256 knots
        FP8_8::rightShiftAndConstruct(gpsSpeed_,8));
  }
  sail::Angle<short> gpsBearing() const {
    return sail::Angle<short>::degrees(gpsBearing_);  // degrees
  }
  char hour() const {
    return hour_;
  }
  char min() const {
    return min_;
  }
  char sec() const {
    return sec_;
  }
  char day() const {
    return day_;
  }
  char month() const {
    return month_;
  }
  char year() const {
    return year_;
  }

#ifdef ON_SERVER
  sail::TimeStamp timestamp() const {
    return sail::TimeStamp::tryUTC(
        year() + 2000, month(), day(),
        hour(), min(), sec());
  }

  std::string sentenceType() const;
#endif

  // Wind data
  sail::Angle<short> awa() const {
    return sail::Angle<short>::degrees(awa_);  // aparent wind angle [degrees]
  }
  sail::Angle<short> twa() const {
    return sail::Angle<short>::degrees(twa_);  // true wind angle [degrees]
  }
  sail::Velocity<FP8_8> aws() const {
    // apparent wind speed [1/256 knots]
    return sail::Velocity<FP8_8>::knots(
        // 1/256 knots
        FP8_8::rightShiftAndConstruct(aws_, 8));
  }
  sail::Velocity<FP8_8> tws() const {
    // true wind speed [1/256 knots]
    return sail::Velocity<FP8_8>::knots(
        // 1/256 knots
        FP8_8::rightShiftAndConstruct(tws_, 8));
  }
  sail::Angle<short> magHdg() const {
    // magnetic heading [degrees]
    return sail::Angle<short>::degrees(magHdg_);
  }
  sail::Velocity<FP8_8> watSpeed() const {
    // water speed [1/256 knots]
    return sail::Velocity<FP8_8>::knots(
        FP8_8::rightShiftAndConstruct(watSpeed_, 8));
  }
  DWord cwd() const {
    return cwd_;  // cumulative water distance [nautical miles]
  }
  DWord wd() const {
    return wd_;  // water distance since reset [1/10 nautical miles]
  }

  sail::Length<double> watDist() const {
    return sail::Length<double>::nauticalMiles(wd_ / 10.0);
  }

  DWord numBytes() const {
    return numBytes_;
  }
  Word numErr() const {
    return numErr_;
  }
  Word numSentences() const {
    return numSentences_;
  }

  bool ignoreWrongChecksum() const {
    return ignoreWrongChecksum_;
  }
  void setIgnoreWrongChecksum(bool val) {
    ignoreWrongChecksum_ = val;
  }

#ifdef ON_SERVER
  std::string awaAsString() const;
  std::string gpsSpeedAsString() const;
  std::string awsAsString() const;
  std::string twaAsString() const;
  std::string twsAsString() const;
  std::string magHdgAsString() const;
  std::string watSpeedAsString() const;
  std::string gpsBearingAsString() const;
  std::string cwdAsString() const;
  std::string wdAsString() const;
#endif

  virtual ~NmeaParser() {}
 protected:
  // XDR,A,-25.8,D,RUDDER
  virtual void onXDRRudder(const char *senderAndSentence,
                           bool valid,
                           sail::Angle<double> angle,
                           const char *whichRudder) { }
  virtual void onXDRPitch(const char *senderAndSentence,
                          bool valid,
                          sail::Angle<double> angle) { }
  virtual void onXDRRoll(const char *senderAndSentence,
                         bool valid,
                         sail::Angle<double> angle) { }

  // The direction from which the wind blows across the earth’s surface, with
  // respect to north, and the speed of the wind.
  // $IIMWD,,T,281.6,M,6.78,N,3.49,M*60
  virtual void onMWD(const char *senderAndSentence,
                     Optional<sail::Angle<>> twdir_geo,
                     Optional<sail::Angle<>> twdir_mag,
                     Optional<sail::Velocity<>> tws) { }
  virtual void onRSA(const char *senderAndSentence,
                     Optional<sail::Angle<>> rudderAngle0,
                     Optional<sail::Angle<>> rudderAngle1) { }
  virtual void onHDM(const char *senderAndSentence,
                     sail::Angle<> magHdg) { }


 private:
  enum NPState {
    NP_STATE_SOM, 	        // Search for start of message
    NP_STATE_CMD,		// Get command and args
    NP_STATE_CHECKSUM_1,	// Get first checksum character
    NP_STATE_CHECKSUM_2,	// get second checksum character
    NP_STATE_IMPL_EOS,	// implicit end of sentence
  };

  // GPS data
  Word gpsSpeed_; // 1/256 knots
  short gpsBearing_; // degrees
  char hour_, min_, sec_;
  char day_, month_, year_;

  // Wind data
  short awa_; // aparent wind angle [degrees]
  short twa_; // true wind angle [degrees]
  Word aws_; // apparent wind speed [1/256 knots]
  Word tws_; // true wind speed [1/256 knots]
  Word magHdg_; // magnetic heading [degrees]
  Word watSpeed_; // water speed [1/256 knots]
  GeoPos pos_;
  DWord cwd_; // cumulative water distance [nautical miles]
  DWord wd_; // water distance since reset [1/10 nautical miles]

  NPState state_;
  Byte checksum_;
  int index_;
  char allow_implicit_eos_;

  char data_[NP_MAX_DATA_LEN];
  char *argv_[NP_MAX_ARGS];
  Byte argc_;

  Byte receivedChecksum_;
  DWord numBytes_;
  Word numErr_;
  Word numSentences_;

  bool ignoreWrongChecksum_;

  char computeChecksum() const;

  NmeaSentence processCommand();
  NmeaSentence processGPRMC();
  NmeaSentence processMWV();
  NmeaSentence processVWR();
  NmeaSentence processVWT();
  NmeaSentence processVHW();
  NmeaSentence processVLW();
  NmeaSentence processGLL();
  NmeaSentence processZDA();
  NmeaSentence processVTG();
  NmeaSentence processXDR();
  NmeaSentence processHDM();
  NmeaSentence processMWD();
  NmeaSentence processRSA();
};

double geoPosDist(GeoPos *a, GeoPos *b);
void secToStr(int sec, char *str);

#endif
