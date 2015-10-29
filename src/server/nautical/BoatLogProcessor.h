/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATLOGPROCESSOR_H_
#define BOATLOGPROCESSOR_H_

#include <server/nautical/Nav.h>
#include <Poco/Path.h>

namespace sail {


int mainProcessBoatLogs(int argc, const char **argv);

bool processBoatDataFullFolder(bool debug, Poco::Path dataPath);

} /* namespace sail */

#endif /* BOATLOGPROCESSOR_H_ */
