/*
 *  Created on: May 13, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef BOATLOGPROCESSOR_H_
#define BOATLOGPROCESSOR_H_

#include <Poco/Util/Application.h>
#include <server/nautical/Nav.h>
#include <Poco/Path.h>

namespace sail {

class TargetSpeedData;
class Calibrator;
void computeTargetSpeedData(Calibrator &c, TargetSpeedData *uw, TargetSpeedData *dw);

// This class implements a command line interface.
class BoatLogProcessor : public Poco::Util::Application {
 public:
  BoatLogProcessor() : Application() {}
  BoatLogProcessor(int argc, char **argv) : Application(argc, argv) {}
  int main(const std::vector<std::string>& args);
 private:
};

void processBoatDataFullFolder(Poco::Path dataPath);

// Implements this:
// https://github.com/jpilet/anemomind-web/pull/9#discussion_r12632698
void processBoatDataSingleLogFile(Poco::Path dataPath, std::string logFilename);

// This function does the processing.
void processBoatDataFullFolder(Nav::Id boatId, Poco::Path srcPath, Poco::Path dstPath);

} /* namespace sail */

#endif /* BOATLOGPROCESSOR_H_ */
