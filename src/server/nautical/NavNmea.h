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


namespace sail {


// Class to hold the results when reading Navs from a file
class ParsedNavs {
 public:
  static const int FIELD_COUNT = 8;
  enum FieldId {TIME = 0, POS, AWA, AWS, MAG_HDG, GPS_BEARING, GPS_SPEED, WAT_SPEED};
  typedef std::bitset<FIELD_COUNT> FieldMask;

  // To let us write, for instance, field(AWA) | field(AWS) to construct a mask
  // where the fields AWA and AWS are set.
  static FieldMask field(FieldId f) {
    FieldMask mask;
    mask.set(f, true);
    return mask;
  }

  ParsedNavs() {}
  ParsedNavs(Array<Nav> navs, FieldMask fields) : _navs(navs), _fields(fields) {}

  Array<Nav> navs() {return _navs;}

  bool hasFields(FieldMask mask);
  bool complete() {return _fields.all();}
 protected:
  Array<Nav> _navs;
  FieldMask _fields;
};

ParsedNavs loadNavsFromNmea(std::istream &file);
ParsedNavs loadNavsFromNmea(std::string filename);



} /* namespace sail */

#endif /* NAVNMEA_H_ */
