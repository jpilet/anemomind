/*
 * v2demo.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/Processor2.h>
#include <server/common/AbstractArray.h>

using namespace sail;

int main(int argc, const char **argv) {
  LogLoader loader;
  loader.load(PathBuilder::makeDirectory(Env::SOURCE_DIR)
    .pushDirectory("datasets")
    .pushDirectory("AlinghiGC32")
    .pushDirectory("logs").get().toString());
  auto ds = loader.makeNavDataset();

  Processor2::Settings settings;
  auto timeStamps = Processor2::getAllGpsTimeStamps(ds.dispatcher().get());

  std::cout << "Number of time stamps: "<< timeStamps.size() << std::endl;

  auto subSessionSpans =
      Processor2::segmentSubSessions(timeStamps, settings);

  Processor2::outputTimeSpansToFile(
      settings.makeLogFilename("subsessions.txt"),
      subSessionSpans);

  auto gpsFilterSpans = Processor2::groupSessionsByThreshold(
      subSessionSpans, settings.mainSessionCut);

  Processor2::outputGroupsToFile(
      settings.makeLogFilename("gpsfiltergroups.txt"),
      gpsFilterSpans, subSessionSpans);


  auto calibGroups = Processor2::computeCalibrationGroups(
      subSessionSpans, settings.minCalibDur);

  Processor2::outputGroupsToFile(
        settings.makeLogFilename("calibgroups.txt"),
        calibGroups,
        subSessionSpans);

  return 0;
}


