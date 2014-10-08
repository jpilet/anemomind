#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>

#include <algorithm>
#include <server/common/string.h>

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

ScreenInfo ScreenRecordingSimulator::screenAt(TimeStamp time) {
  sort();
  ScreenInfo wanted;
  wanted.time = time;
  wanted.perf = -1;
  wanted.twdir = -1;
  wanted.tws = -1;

  auto justAfter = lower_bound(_screenInfo.begin(), _screenInfo.end(), wanted);
  if (justAfter == _screenInfo.end() || justAfter == _screenInfo.begin()) {
    return wanted;
  }
  auto atResult = justAfter - 1;
  return *atResult;
}

void ScreenRecordingSimulator::simulate(const std::vector<std::string>& nmeaFiles,
                const char* boatDatFilename,
                const char* polarDatFilename) {
  if (boatDatFilename) {
    SD()->setReadableFile("boat.dat", readFileToString(boatDatFilename));
  }

  if (polarDatFilename) {
    SD()->setReadableFile("polar.dat", readFileToString(polarDatFilename));
  }

  setup();

  for (auto file : nmeaFiles) {
    sendData(readFileToString(file));
  }
}

}  // namespace sail
