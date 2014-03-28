/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmea.h"
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>


namespace sail {

namespace {
  const char *getNmeaSentenceLabel(NmeaParser::NmeaSentence s) {
    typedef const char *Str;
    Str labels[] = {"0", //"NMEA_NONE",
        "NMEA_UNKNOWN",
        "NMEA_TIME_POS",
        "NMEA_AW",
        "NMEA_TW",
        "NMEA_WAT_SP_HDG",
        "NMEA_VLW"};
    return labels[s];
  }


  TimeStamp getTime(const NmeaParser &parser) {
    return NavDataConversion::makeTimeNmeaFromYMDhms(parser.year(), parser.month(), parser.day(),
                                                 parser.hour(), parser.min(), parser.sec());
  }

  Angle<double> getGpsBearing(const NmeaParser &parser) {
    return Angle<double>::degrees(parser.gpsBearing());
  }

  Velocity<double> getGpsSpeed(const NmeaParser &parser) {
    return Velocity<double>::knots(parser.gpsSpeed());
  }

  Angle<double> getAngle(const AccAngle &x) {
    return Angle<double>::degMinMc(x.deg(), x.min(), x.mc());
  }

  Angle<double> getLon(const NmeaParser &parser) {
    return getAngle(parser.pos().lon);
  }

  Angle<double> getLat(const NmeaParser &parser) {
    return getAngle(parser.pos().lat);
  }

  GeographicPosition<double> getGeoPos(const NmeaParser &parser) {
    return GeographicPosition<double>(getLon(parser), getLat(parser));
  }

  // processGPRMC
  void readNmeaTimePos(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setTime(getTime(parser));
    mask->set(ParsedNavs::TIME, true);

    dstNav->setGpsBearing(getGpsBearing(parser));
    mask->set(ParsedNavs::GPS_BEARING, true);

    dstNav->setGpsSpeed(getGpsSpeed(parser));
    mask->set(ParsedNavs::GPS_SPEED);

    dstNav->setGeographicPosition(getGeoPos(parser));
    mask->set(ParsedNavs::POS);
  }

  Angle<double> getAwa(const NmeaParser &parser) {
    return Angle<double>::degrees(parser.awa());
  }

  Velocity<double> getAws(const NmeaParser &parser) {
    return Velocity<double>::knots(parser.aws());
  }


  void readNmeaAW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setAwa(getAwa(parser));
    mask->set(ParsedNavs::AWA, true);

    dstNav->setAws(getAws(parser));
    mask->set(ParsedNavs::AWS, true);
  }

  void readNmeaTW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    // Ignore the true wind. We will calculate it ourselves.
  }

  Velocity<double> getWatSpeed(const NmeaParser &parser) {
    return Velocity<double>::knots(parser.watSpeed());
  }

  Angle<double> getMagHdg(const NmeaParser &parser) {
    return Angle<double>::degrees(parser.magHdg());
  }

  void readNmeaWatSpHdg(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setWatSpeed(getWatSpeed(parser));
    mask->set(ParsedNavs::WAT_SPEED, true);

    dstNav->setMagHdg(getMagHdg(parser));
    mask->set(ParsedNavs::MAG_HDG, true);
  }

  void readNmeaVLW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    // Ignored
  }

  void readNmeaData(NmeaParser::NmeaSentence s,
        const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    switch (s) {
     case NmeaParser::NMEA_TIME_POS:
       readNmeaTimePos(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_AW:
       readNmeaAW(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_TW:
       readNmeaTW(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_WAT_SP_HDG:
       readNmeaWatSpHdg(parser, dstNav, mask);
       break;
     case NmeaParser::NMEA_VLW:
       readNmeaVLW(parser, dstNav, mask);
       break;
     default:
       {
         //LOG(INFO) << stringFormat("Sentence not supported: %s", getNmeaSentenceLabel(s));
       }
       break;
    };
  }
}



bool ParsedNavs::hasFields(FieldMask mask) {
  // a implies b   <->  (not a) or b
  // It should always be true that when a bit is set in mask,
  // it implies the corresponding bit is set in _fields.
  return (~mask | _fields).all();
}

namespace {
  bool parseNmeaChar(std::istream &file,
      NmeaParser *parser, Nav *dstNav, std::vector<Nav> *navAcc, ParsedNavs::FieldMask *fields) {
    if (file.good()) {
      char c;
      file.get(c);
      NmeaParser::NmeaSentence s = parser->processByte(c);
      if (s != NmeaParser::NMEA_NONE) {
        readNmeaData(s, *parser, dstNav, fields);
        // Once a time stamp has been received, save this Nav and start to fill a new one.
        if (s == NmeaParser::NMEA_TIME_POS) {
          navAcc->push_back(*dstNav);
          *dstNav = Nav();
        }
      }
      return true;
    }
    return false;
  }
}

ParsedNavs loadNavsFromNmea(std::istream &file) {
  ParsedNavs::FieldMask fields;
  std::vector<Nav> navAcc;
  NmeaParser parser;
  parser.setIgnoreWrongChecksum(true);
  std::string line;
  int lineCounter = 0;
  Nav nav;
  while (parseNmeaChar(file, &parser, &nav, &navAcc, &fields)) {
    // Empty
  }
  return ParsedNavs(Array<Nav>::referToVector(navAcc).dup(), fields);
}

ParsedNavs loadNavsFromNmea(std::string filename) {
  std::ifstream file(filename);
  return loadNavsFromNmea(file);
}



} /* namespace sail */
