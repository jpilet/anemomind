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

void processBoatDataFullFolder(bool debug, Poco::Path dataPath);

// Implements this:
// https://github.com/jpilet/anemomind-web/pull/9#discussion_r12632698
void processBoatDataSingleLogFile(bool debug, Poco::Path dataPath, std::string logFilename);

// This function does the processing.
void processBoatDataFullFolder(bool debug, Nav::Id boatId, Poco::Path srcPath, Poco::Path dstPath);

} /* namespace sail */

#endif /* BOATLOGPROCESSOR_H_ */
