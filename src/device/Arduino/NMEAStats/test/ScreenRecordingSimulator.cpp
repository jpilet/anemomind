#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>

#include <algorithm>
#include <server/common/string.h>
#include <server/common/logging.h>

namespace sail {

void ScreenRecordingSimulator::screenUpdate(int perf, int twdir, int tws) {
  ScreenInfo info;
  info.time = getTimeStamp();
  info.perf = perf;
  info.twdir = twdir;
  info.tws = tws;
  _screenInfo.push_back(info);
  _sorted = false;
}

void ScreenRecordingSimulator::sort() {
  if (!_sorted) {
    std::sort(_screenInfo.begin(), _screenInfo.end());
    _sorted = true;
  }
}

bool ScreenRecordingSimulator::screenAt(TimeStamp time, ScreenInfo *result) {
  sort();
  result->time = time;
  result->perf = -1;
  result->twdir = -1;
  result->tws = -1;

  auto justAfter = lower_bound(_screenInfo.begin(), _screenInfo.end(), *result);
  if (justAfter == _screenInfo.end() || justAfter == _screenInfo.begin()) {
    return false;
  }
  auto atResult = justAfter - 1;
  *result = *atResult;
  return true;
}

bool ScreenRecordingSimulator::prepare(
                const std::string& boatDatFilename,
                const std::string& polarDatFilename) {
  if (boatDatFilename.size() > 0) {
    SD()->setReadableFile("boat.dat", readFileToString(boatDatFilename));
  }

  if (polarDatFilename.size() > 0) {
    SD()->setReadableFile("polar.dat", readFileToString(polarDatFilename));
  }

  setup();
  if (boatDatFilename.size() > 0) {
    if (!calibrationFileLoaded()) {
	    return false;
    }
    //CHECK(calibrationFileLoaded())
    //  << "Failed to load " << boatDatFilename << " in the simulated device.";
  }

  if (polarDatFilename.size() > 0) {
	  return false;
    CHECK(polarTableLoadedOrDisabled())
      << "Failed to load " << polarDatFilename << " in the simulated device.";
  }
  return true;
}

void ScreenRecordingSimulator::simulate(std::string file) {
    sendData(readFileToString(file));
}

void ScreenRecordingSimulator::simulate(const std::vector<std::string>& nmeaFiles,
                const char* boatDatFilename,
                const char* polarDatFilename) {
  prepare(boatDatFilename, polarDatFilename);

  for (auto file : nmeaFiles) {
    sendData(readFileToString(file));
  }
}

}  // namespace sail
