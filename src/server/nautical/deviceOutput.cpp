// Julien Pilet, 2014.
//
// Tool to process a nmea log through a simulated device. Produces a json file
// containing an array of:
// {timestamp, perf, twdir, tws} as would have beend displayed by the device.
// The array is sorted.

#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>
#include <server/nautical/logimport/LogLoader.h>
#include <stdio.h>

using namespace sail;

namespace {
bool save(const char* filename, ScreenRecordingSimulator *simulator) {
  FILE *file = fopen(filename, "wt");
  if (!file) {
    return false;
  }

  simulator->sort();

  fprintf(file, "[\n");
  bool first = true;
  for (const ScreenInfo& info : simulator->screenInfo()) {
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
  const char* polarDatFilename = nullptr;
  const char* outputFilename = "display.js";
  std::vector<std::string> inputFiles;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if ((i + 1) < argc && argv[i][1] == 'd') {
        ++i;
        boatDatFilename = argv[i];
      } else if ((i + 1) < argc && argv[i][1] == 'p') {
        ++i;
        polarDatFilename = argv[i];
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

  arduino.simulate(inputFiles, boatDatFilename, polarDatFilename);

  if (!save(outputFilename, &arduino)) {
    perror(outputFilename);
  }

  return 0;
}
