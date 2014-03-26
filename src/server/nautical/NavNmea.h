/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Read navs from Nmea
 */

#ifndef NAVNMEA_H_
#define NAVNMEA_H_

#include <server/nautical/Nav.h>

namespace sail {

Array<Nav> loadNavsFromNmea(std::istream &file);
Array<Nav> loadNavsFromNmea(std::string filename);

} /* namespace sail */

#endif /* NAVNMEA_H_ */
