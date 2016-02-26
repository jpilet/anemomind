/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmea.h"
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <fstream>
#include <server/common/ArrayBuilder.h>
#include <iostream>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <algorithm>


namespace sail {

using namespace NavCompat;

namespace {
  Angle<double> getGpsBearing(const NmeaParser &parser) {
    return Angle<double>(parser.gpsBearing());
  }

  Velocity<double> getGpsSpeed(const NmeaParser &parser) {
    return Velocity<double>(parser.gpsSpeed());
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
    return Angle<double>(parser.awa());
  }

  Velocity<double> getAws(const NmeaParser &parser) {
    return Velocity<double>(parser.aws());
  }


  void readNmeaAW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setAwa(getAwa(parser));
    mask->set(ParsedNavs::AWA, true);

    dstNav->setAws(getAws(parser));
    mask->set(ParsedNavs::AWS, true);
  }

  void readNmeaTW(const NmeaParser &parser, Nav *dstNav, ParsedNavs::FieldMask *mask) {
    dstNav->setExternalTwa(parser.twa());
    mask->set(ParsedNavs::TWA_EXTERNAL, true);
    dstNav->setExternalTws(parser.tws());
    mask->set(ParsedNavs::TWS_EXTERNAL, true);
  }

  Velocity<double> getWatSpeed(const NmeaParser &parser) {
    return Velocity<double>(parser.watSpeed());
  }

  Angle<double> getMagHdg(const NmeaParser &parser) {
    return Angle<double>(parser.magHdg());
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
       break;
    };
  }
}

// TODO: move this function to somewhere that makes sense.
// and remove duplicate in Nmea0183Source.cpp
TimeStamp getTime(const NmeaParser &parser) {
  return NavDataConversion::makeTimeNmeaFromYMDhms(
      parser.year(), parser.month(), parser.day(),
      parser.hour(), parser.min(), parser.sec());
}

bool ParsedNavs::hasFields(FieldMask mask) {
  // a implies b   <->  (not a) or b
  // It should always be true that when a bit is set in mask,
  // it implies the corresponding bit is set in _fields.
  return (~mask | _fields).all();
}

ParsedNavs::FieldMask ParsedNavs::makeCompleteMask() {
  FieldMask fm;
  fm.flip();
  return fm;
}

ParsedNavs::FieldMask ParsedNavs::makeGpsWindMask() {
    FieldMask fm;
    fm[POS] = fm[TIME] = fm[AWA] = fm[AWS] = true;
    return fm;
}

ParsedNavs::FieldMask ParsedNavs::makeAllSensorsMask() {
    FieldMask fm;
    fm[POS] = fm[TIME] = fm[AWA] = fm[AWS] = fm[MAG_HDG] = fm[GPS_SPEED] = fm[WAT_SPEED] = true;
    return fm;
}
    
namespace {
  bool hasGreaterTimeStamp(ArrayBuilder<Nav> *navAcc, Nav *nav) {
    if (navAcc->empty()) {
      return true;
    }
    return navAcc->last().time() < nav->time();
  }

  void parseNmeaChar(char c,
    NmeaParser *parser, Nav *dstNav, ArrayBuilder<Nav> *navAcc,
    ParsedNavs::FieldMask *fields, Nav::Id boatId,
    TimeStamp *last) {
    NmeaParser::NmeaSentence s = parser->processByte(c);

    const Duration<double> maxDurationBetweenTimeMeasures
      = Duration<double>::minutes(2); // <-- TODO: Maybe soft-code this threshold in future...

    if (s != NmeaParser::NMEA_NONE) {
      readNmeaData(s, *parser, dstNav, fields);
      // Once a time stamp has been received, save this Nav and start to fill a new one.
      if (s == NmeaParser::NMEA_TIME_POS) {
        TimeStamp veryLast = dstNav->time();

        // Don't accept measurements that could potentially be arbitrarily old...
        if (last->defined() &&

          // Only accept measurements that are guaranteed to
          // be sufficiently fresh, e.g. no more than two minutes.
          ((veryLast - *last) <= maxDurationBetweenTimeMeasures)) {

          dstNav->setBoatId(boatId);
          navAcc->add(*dstNav);
        } else {
          // Note that we reset dstNav only if there was a time gap.
          // This propagates fresh values to the next Nav. 
          // It makes sense, since NMEA is a stream of data combining
          // instruments working at different frequencies.
          *dstNav = Nav();
        }
        *last = veryLast;
      }
    }
  }
}

ParsedNavs loadNavsFromNmea(std::istream &file, Nav::Id boatId) {
  ParsedNavs::FieldMask fields;
  ArrayBuilder<Nav> navAcc;
  NmeaParser parser;
  parser.setIgnoreWrongChecksum(true);
  std::string line;
  Nav nav;
  TimeStamp last;
  while (file.good()) {
    char c;
    file.get(c);
    parseNmeaChar(c, &parser, &nav, &navAcc, &fields, boatId, &last);
  }
  return ParsedNavs(fromNavs(navAcc.get()), fields);
}

ParsedNavs loadNavsFromNmea(std::string filename, Nav::Id boatId) {
  std::ifstream file(filename);
  return loadNavsFromNmea(file, boatId);
}

namespace {

std::string getFieldLabel(ParsedNavs::FieldId id) {
  switch (id) {
    case ParsedNavs::TIME: return "TIME";
    case ParsedNavs::POS: return "POS";
    case ParsedNavs::AWA: return "AWA";
    case ParsedNavs::AWS: return "AWS";
    case ParsedNavs::MAG_HDG: return "MAG_HDG";
    case ParsedNavs::GPS_BEARING: return "GPS_BEARING";
    case ParsedNavs::GPS_SPEED: return "GPS_SPEED";
    case ParsedNavs::WAT_SPEED: return "WAT_SPEED";
    case ParsedNavs::TWA_EXTERNAL: return "TWA_EXTERNAL";
    case ParsedNavs::TWS_EXTERNAL: return "TWS_EXTERNAL";
    case ParsedNavs::FIELD_COUNT: return "UNKNOWN FIELD";
  }
}

} // namespace

std::ostream &operator<<(std::ostream &s, ParsedNavs x) {
  s << "ParsedNavs: " << getNavSize(x.navs()) << std::endl;
  for (int i = 0; i < ParsedNavs::FIELD_COUNT; i++) {
    ParsedNavs::FieldId id = ParsedNavs::FieldId(i);
    s << "   " << getFieldLabel(id) << ": "
        << x.hasFields(ParsedNavs::field(id)) << std::endl;
  }
  return s;
}

namespace {
  int countNavsToInclude(Array<ParsedNavs> allNavs, ParsedNavs::FieldMask mask) {
    int counter = 0;
    for (int i = 0; i < allNavs.size(); i++) {
      if (allNavs[i].hasFields(mask)) {
        counter += getNavSize(allNavs[i].navs());
      }
    }
    return counter;
  }

  NavDataset flatten(Array<ParsedNavs> allNavs, ParsedNavs::FieldMask mask) {
    int len = countNavsToInclude(allNavs, mask);
    Array<Nav> dst(len);
    int counter = 0;
    for (int i = 0; i < allNavs.size(); i++) {
      ParsedNavs &n = allNavs[i];
      if (n.hasFields(mask)) {
        int next = counter + getNavSize(n.navs());
        makeArray(n.navs()).copyToSafe(dst.slice(counter, next));
        counter = next;
      }
    }
    assert(counter == dst.size());
    return fromNavs(dst);
  }
}

NavDataset flattenAndSort(Array<ParsedNavs> allNavs, ParsedNavs::FieldMask mask) {
  NavDataset flattened = flatten(allNavs, mask);

  LOG(WARNING) << "Take another look at this code.";
  auto copy = makeArray(flattened);

  std::sort(copy.begin(), copy.end());
  return fromNavs(copy);
}

ParsedNavs::FieldMask ParsedNavs::fieldsFromNavs(const NavDataset &navs) {
  FieldMask result;
  for (auto nav : Range(navs)) {
    if (nav.hasApparentWind()) {
      result.set(FieldId::AWA);
      result.set(FieldId::AWS);
    }
    if (nav.hasExternalTrueWind()) {
      result.set(FieldId::TWA_EXTERNAL);
      result.set(FieldId::TWS_EXTERNAL);
    }
    if (nav.hasWatSpeed()) {
      result.set(FieldId::WAT_SPEED);
    }
    if (nav.hasMagHdg()) {
      result.set(FieldId::MAG_HDG);
    }
  }
  // Nav does not have hasGps(), we assume we do have time and pos.
  if (getNavSize(navs) > 0) {
    result.set(FieldId::TIME);  
    result.set(FieldId::POS);  
    result.set(FieldId::GPS_BEARING);  
    result.set(FieldId::GPS_SPEED);  
  }
  return result;
}

} /* namespace sail */
