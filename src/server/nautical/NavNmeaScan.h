/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NAVNMEASCAN_H_
#define NAVNMEASCAN_H_

#include <Poco/Path.h>
#include <server/nautical/Nav.h>
#include <server/nautical/NavNmea.h>

namespace sail {

class ScreenRecordingSimulator;

Array<Nav> scanNmeaFolderWithSimulator(Poco::Path p, Nav::Id boatId,
                          ScreenRecordingSimulator* simulator = 0,
                          ParsedNavs::FieldMask mask = ParsedNavs::makeGpsWindMask());

Array<Nav> scanNmeaFolder(Poco::Path p, Nav::Id boatId,
    ParsedNavs::FieldMask mask = ParsedNavs::makeAllSensorsMask());

Array<Nav> scanNmeaFolders(Array<Poco::Path> p, Nav::Id boatId,
    ParsedNavs::FieldMask mask = ParsedNavs::makeAllSensorsMask());

} /* namespace sail */

#endif /* NAVNMEASCAN_H_ */
