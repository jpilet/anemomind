/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Read navs from Nmea
 */

#ifndef NAVNMEA_H_
#define NAVNMEA_H_

#include <server/nautical/Nav.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>

namespace sail {

// Class to hold the results of a parsed Nmea file.
class NavNmeaData {
 public:
  NavNmeaData() {}
  NavNmeaData(Array<Nav> n, Arrayb f) : _navs(n), _sentencesReceived(f) {}
  NavNmeaData(std::istream &file);
  NavNmeaData(std::string filename);

  Array<Nav> navs() {return _navs;}
 private:
  Array<Nav> _navs;
  Arrayb _sentencesReceived;
  void init(std::istream &file);
};

} /* namespace sail */

#endif /* NAVNMEA_H_ */
