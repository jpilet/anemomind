#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>

#include <algorithm>
#include <server/common/string.h>

  #include <iostream>

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

void ScreenRecordingSimulator::prepare(
                const std::string& boatDatFilename,
                const std::string& polarDatFilename) {


  std::cout << EXPR_AND_VAL_AS_STRING(polarDatFilename) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(boatDatFilename) << std::endl;

  if (boatDatFilename.size() > 0) {
    SD()->setReadableFile("boat.dat", readFileToString(boatDatFilename));
  }

  std::cout << EXPR_AND_VAL_AS_STRING(polarDatFilename.size()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(polarDatFilename.length()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(polarDatFilename.empty()) << std::endl;

  if (polarDatFilename.size() > 0) {
    std::string polarString = readFileToString(polarDatFilename);
    std::cout << EXPR_AND_VAL_AS_STRING(polarString) << std::endl;
    SD()->setReadableFile("polar.dat", polarString);
  }

  setup();
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
