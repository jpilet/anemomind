/*
 * tryNewGpsFilterOnAnyData.cpp
 *
 *  Created on: Jun 9, 2016
 *      Author: jonas
 */

#include <fstream>
#include <server/common/ArgMap.h>
#include <server/common/PathBuilder.h>
#include <server/common/logging.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/segment/SessionCut.h>
#include <server/common/DOMUtils.h>

using namespace sail;


Array<TimeStamp> getTimeStamps(const NavDataset &ds) {
  ArrayBuilder<TimeStamp> builder;
  for (auto sample: ds.samples<GPS_POS>()) {
    builder.add(sample.time);
  }
  return builder.get();

}

void outputLocalPositions(const std::string &filename,
    const Array<CeresTrajectoryFilter::Types<2>::TimedPosition> &positions) {
  std::ofstream file(filename);

  file << "% Matlab-loadable file: First column epoch (time in ms since 1970), "
      "second column X coordinate in meters, third column Y coordinate in meters\n";

  for (auto p: positions) {
    file << p.time.toMilliSecondsSince1970()
        << " " << p.value[0].meters()
        << " " << p.value[1].meters() << "\n";
  }
}

int performTheFiltering(std::string inputPath, std::string outputPath,
    const SessionCut::Settings &cutSettings) {
  PathBuilder pb = PathBuilder::makeDirectory(outputPath);

  LOG(INFO) << "Loading " << inputPath << "...";
  LogLoader loader;
  loader.load(inputPath);
  auto fullDataset = loader.makeNavDataset().fitBounds();
  LOG(INFO) << "Cut sessions...";
  auto sessions = SessionCut::cutSessions(wrapIndexable<TypeMode::Value>(
      getTimeStamps(fullDataset)), cutSettings);

  {
    std::ofstream file(pb.makeFile("sessions.txt").get().toString());
    for (int i = 0; i < sessions.size(); i++) {
      auto s = sessions[i];
      file << "Session " << i << " from " << s.minv() << " to " << s.maxv() << std::endl;
      if (i < sessions.size() - 1) {
        auto next = sessions[i+1];
        file << "\n\nGap of " << (next.minv() - s.maxv()).str() << std::endl;
      }
    }
  }
  LOG(INFO) << "Got "<< sessions.size() << " sessions";

  for (int i = 0; i < sessions.size(); i++) {
    auto session = sessions[i];
    auto ds = fullDataset.slice(session.minv(), session.maxv());
    LOG(INFO) << "Filtering session from " << session.minv() << " to " << session.maxv();
    DOM::Node output;
    auto filtered = filterGpsData(ds, &output);
    LOG(INFO) << "Filtered, save it";
    outputLocalPositions(pb.makeFile(stringFormat("raw_positions_%d.txt", i)).get().toString(),
        filtered.rawLocalPositions);
    outputLocalPositions(pb.makeFile(stringFormat("filtered_positions_%d.txt", i)).get().toString(),
        filtered.filteredLocalPositions);
    LOG(INFO) << "Saved";
  }
  LOG(INFO) << "Filtered all sessions";
  return 0;
}

int main(int argc, const char **argv) {

  std::string path;
  std::string outputPath = "/tmp/";

  SessionCut::Settings cutSettings;

  ArgMap amap;
  amap.setHelpInfo("Example usage:\n./filters_tryNewGpsFilter --path /home/jonas/data/datasets/boat55a774ac16361494ab094dc7/ --segment-reg 1000");
  amap.registerOption("--path", "Provide a path to the log files to be filtered")
      .setArgCount(1)
      .store(&path).setRequired();

  amap.registerOption("--output", "path to output to")
      .store(&outputPath);

  amap.registerOption("--segment-reg", "How the segmentation should be regularized")
      .store(&(cutSettings.regularization));

  auto status = amap.parse(argc, argv);
  switch (status) {
  case ArgMap::Continue:
    return performTheFiltering(path, outputPath, cutSettings);
  case ArgMap::Done:
    return 0;
  case ArgMap::Error:
    return -1;
  };
  return 0;
}
