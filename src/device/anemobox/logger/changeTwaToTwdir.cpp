/*
 * To get the internal GPS NMEA stream, use:
 * ./anemobox_logcat -t "Internal GPS NMEA" <logfile>
 */

#include <device/anemobox/logger/Logger.h>
#include <device/anemobox/logger/logger.pb.h>
#include <server/common/ArgMap.h>
#include <server/common/logging.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace sail;
using namespace std;

namespace {

void swapTwaTwdir(const string& file) {
  LogFile data;
  if (Logger::read(file, &data)) {

    int numChanged = 0;
    for (int i = 0; i < data.stream_size(); ++i) {
      if (data.stream(i).shortname() == "twa") {
        data.mutable_stream(i)->set_shortname("twdir");
        ++numChanged;
      }
    }
    if (numChanged > 0) {
      LOG(INFO) << file << ": changed " << numChanged << " streams.";
      Logger::save(file, data);
    }
  }
}

}  // namespace

int main(int argc, const char* argv[]) {
  if (argc <= 1) {
    LOG(FATAL) << "Usage: " << argv[0] << " <logfile> [<logfile> ...]";
  }

  ArgMap cmdLine;
  string textField;

  cmdLine.registerOption("-t", "output only the given text stream")
    .store(&textField)
    .setUnique();

  if (cmdLine.parse(argc, argv) != ArgMap::Continue) {
    return -1;
  }

  for (auto arg : cmdLine.freeArgs()) {
    swapTwaTwdir(arg->value());
  }
  return 0;
}

