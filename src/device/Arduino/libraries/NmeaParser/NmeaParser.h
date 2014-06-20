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

#ifdef NOT_ON_MICROCONTROLLER
#include <string>
#endif

#include "../PhysicalQuantity/PhysicalQuantity.h"
#include "../FixedPoint/FixedPoint.h"

#include <stdint.h>

typedef int32_t DWord;
typedef int16_t Word;
typedef unsigned char Byte;

#define NP_MAX_DATA_LEN 82
#define NP_MAX_ARGS 20

// Represents an accurate angle in fixed point.
// The angle is stored in degrees, minutes and 1/1000 minutes fraction.
class AccAngle {
 public:

  AccAngle();
  AccAngle(double angle) {
    set(angle);
  }
  AccAngle(Word deg, Word min, Word mc) : deg_(deg), min_(min), mc_(mc) { }

  // Returns the angle in degrees.
  double toDouble() const;

  void set(double a);
  void set(Word deg, Word min, Word mc) {
    deg_ = deg;
    min_ = min;
    mc_ = mc;
  }

  Word deg() const {
    return deg_;
  }
  Word min() const {
    return min_;
  }
  Word mc() const {
    return mc_;
  }

  AccAngle &operator = (double a) {
    set(a);
    return *this;
  }

 private:
  Word deg_, min_, mc_;
};

class GeoPos {
 public:
  GeoPos() { }
  GeoPos(int latDeg, int latMin, int latMc, int lonDeg, int lonMin, int lonMc) :
    lon(lonDeg, lonMin, lonMc), lat(latDeg, latMin, latMc) { }

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
    NMEA_TIME_POS,
    NMEA_AW,
    NMEA_TW,
    NMEA_WAT_SP_HDG,
    NMEA_VLW,
  };

  NmeaParser();
  NmeaSentence processByte(Byte data);
  void printSentence();
  void getSentenceString(int bufLen, char *buffer);

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

#ifdef NOT_ON_MICROCONTROLLER
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
  NmeaSentence processVHW();
  NmeaSentence processVLW();
};

double geoPosDist(GeoPos *a, GeoPos *b);
void secToStr(int sec, char *str);

#endif
