/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NAVNMEASCAN_H_
#define NAVNMEASCAN_H_

#include <Poco/Path.h>
#include <server/nautical/Nav.h>

namespace sail {

class ScreenRecordingSimulator;

Array<Nav> scanNmeaFolder(Poco::Path p, Nav::Id boatId,
                          ScreenRecordingSimulator* simulator = 0);

} /* namespace sail */

#endif /* NAVNMEASCAN_H_ */
