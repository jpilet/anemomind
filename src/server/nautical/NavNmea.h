/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Read navs from Nmea
 */

#ifndef NAVNMEA_H_
#define NAVNMEA_H_

#include <server/nautical/Nav.h>
#include <bitset>

class NmeaParser;

namespace sail {


// Class to hold the results when reading Navs from a file.
class ParsedNavs {
 public:
  enum FieldId {
    TIME = 0,
    POS,
    AWA,
    AWS,
    MAG_HDG,
    GPS_BEARING,
    GPS_SPEED,
    WAT_SPEED,
    TWA_EXTERNAL,
    TWS_EXTERNAL,
    FIELD_COUNT
  };
  typedef std::bitset<FIELD_COUNT> FieldMask;

  // To let us write, for instance, field(AWA) | field(AWS) to construct a mask
  // where the fields AWA and AWS are set.
  static FieldMask field(FieldId f) {
    FieldMask mask;
    mask.set(f, true);
    return mask;
  }

  ParsedNavs() {}
  ParsedNavs(NavCollection navs, FieldMask fields) : _navs(navs), _fields(fields) {}
  ParsedNavs(NavCollection navs) : _navs(navs), _fields(fieldsFromNavs(navs)) {}

  NavCollection navs() {return _navs;}

  bool hasFields(FieldMask mask);
  bool complete() { return _fields[TIME] && _fields[POS] && _fields[AWA]; }

  static FieldMask makeCompleteMask();
  static FieldMask makeGpsWindMask();
  static FieldMask makeAllSensorsMask();
  static FieldMask fieldsFromNavs(const NavCollection &navs);

 private:
  NavCollection _navs;

  // Which fields of the Navs for which we read data. For instance,
  // if the magnetic compass was not connected, then we would expect
  // that _fields.test(MAG_HDG) to return false. This lets us discard
  // files with insufficient data.
  FieldMask _fields;
};

ParsedNavs loadNavsFromNmea(std::istream &file, Nav::Id boatId);
ParsedNavs loadNavsFromNmea(std::string filename, Nav::Id boatId);
std::ostream &operator<<(std::ostream &s, ParsedNavs x);

// Useful function when reading Nav's from several files.
NavCollection flattenAndSort(Array<ParsedNavs> allNavs, ParsedNavs::FieldMask mask);

TimeStamp getTime(const NmeaParser &parser);

} /* namespace sail */

#endif /* NAVNMEA_H_ */
