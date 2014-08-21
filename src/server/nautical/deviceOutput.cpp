// Julien Pilet, 2014.
//
// Tool to process a nmea log through a simulated device. Produces a json file
// containing an array of:
// {timestamp, perf, twdir, tws} as would have beend displayed by the device.
// The array is sorted.
#include <device/Arduino/NMEAStats/test/DeviceSimulator.h>

#include <server/common/string.h>
#include <server/nautical/NavNmeaScan.h>
#include <vector>
#include <algorithm>
#include <stdio.h>

using namespace sail;

namespace {

struct ScreenInfo {
  TimeStamp time;
  int perf;
  int twdir;
  int tws;

  bool operator <(const ScreenInfo& other) const {
    return time < other.time;
  }
};

class ScreenRecordingSimulator : public DeviceSimulator {
 public:
  virtual void screenUpdate(int perf, int twdir, int tws);
  bool save(const char* filename);
 private:
  std::vector<ScreenInfo> _screenInfo;
};
      
void ScreenRecordingSimulator::screenUpdate(int perf, int twdir, int tws) {
  ScreenInfo info;
  info.time = getTimeStamp();
  info.perf = perf;
  info.twdir = twdir;
  info.tws = tws;
  _screenInfo.push_back(info);
}

bool ScreenRecordingSimulator::save(const char* filename) {
  FILE *file = fopen(filename, "wt");
  if (!file) {
    return false;
  }

  std::sort(_screenInfo.begin(), _screenInfo.end());

  fprintf(file, "[\n");
  bool first = true;
  for (const ScreenInfo& info : _screenInfo) {
    if (!first) {
      fprintf(file, ",\n");
    }
    first = false;
    fprintf(file, "{time:%lld,perf:%d,twdir:%d,tws:%d}",
            info.time.toMilliSecondsSince1970(),
            info.perf, info.twdir, info.tws);
  }
  fprintf(file, "\n]\n");
  fclose(file);
  return true;
}

void help(const char *exe) {
  fprintf(stderr,
          "usage: %s [-d <boat.dat>] [-o <output>] <nmea file> [<nmea file> ...]\n",
          exe);
}

}  // namespace

int main(int argc, char* argv[]) {
  const char* boatDatFilename = nullptr;
  const char* outputFilename = "display.js";
  std::vector<std::string> inputFiles;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if ((i + 1) < argc && argv[i][1] == 'd') {
        ++i;
        boatDatFilename = argv[i];
      } else if ((i + 1) < argc && argv[i][1] == 'o') {
        ++i;
        outputFilename = argv[i];
      } else {
        help(argv[0]);
        return -1;
      }
    } else {
      inputFiles.push_back(argv[i]);
    }
  }

  if (inputFiles.size() == 0) {
    help(argv[0]);
    return 2;
  }

  ScreenRecordingSimulator arduino;

  if (boatDatFilename) {
    arduino.SD()->setReadableFile(
        "boat.dat", readFileToString(boatDatFilename));
  }

  arduino.setup();

  for (auto file : inputFiles) {
    arduino.sendData(readFileToString(file));
  }

  if (!arduino.save(outputFilename)) {
    perror(outputFilename);
  }

  return 0;
}
