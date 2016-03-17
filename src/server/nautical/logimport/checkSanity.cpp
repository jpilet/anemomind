/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/logimport/LogLoader.h>
#include <server/common/logging.h>

using namespace sail;

namespace {
  Array<const char *> listPathsToLoad(int argc, const char **argv) {
    int n = argc - 1;
    Array<std::string> dst(n);
    if (n == 0) {
      LOG(INFO) << "USAGE: Just pass all the paths that should be loaded, to this program";
    } else {
      for (int i = 0; i < n; i++) {
        auto p = argv[i + 1];
        LOG(INFO) << "  To analyze: " << p;
        dst[i] = p;
      }
    }
    return dst;
  }
}

int main(int argc, const char **argv) {
  auto paths = listPathsToLoad(argc, argv);
  for (int i = 0; i < paths.size(); i++) {
    auto p = paths[i];
    LOG(INFO) << "Processing path " << i + 1 << "/" << paths.size() << ": " << p;
  }
  return 0;
}


