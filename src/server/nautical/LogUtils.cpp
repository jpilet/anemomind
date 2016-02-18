/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <device/anemobox/logger/LogLoader.h>
#include <server/common/filesystem.h>
#include <server/nautical/LogUtils.h>
#include <server/common/logging.h>
#include <server/common/PathBuilder.h>
#include <iostream>
#include <server/nautical/NavLoader.h>

namespace sail {
namespace LogUtils {

LoadStatus loadAll(const Poco::Path &datasetPath, Dispatcher *dst) {
  Array<std::string> extensions{"log"};
  LogLoader loader;
  FileScanSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  int successCounter = 0;
  int attemptCounter = 0;
  forEveryPath(datasetPath, [&](const Poco::Path &p) {
    if (hasExtension(p.toString(), extensions)) {
      attemptCounter++;
      try {
        std::string s = p.toString();
        if (loader.load(p.toString())) {
          successCounter++;
        }
      } catch (const std::exception &e) {
        // See failureCounter = ... later. That's how we detect failures.
      }
    }
  }, settings);
  int failureCounter = attemptCounter - successCounter;
  loader.addToDispatcher(dst);
  return LoadStatus{successCounter, failureCounter};
}

LoadStatus loadAll(const std::string &path, Dispatcher *dst) {
  return loadAll(PathBuilder::makeDirectory(path).get(), dst);
}

std::ostream &operator<<(std::ostream &s, const LoadStatus &x) {
  s << "\nsail::LogUtils::LoadStatus: "
    << "\n  successfully loaded " << x.successCount << " files"
    << "\n  failed to load      " << x.failureCount << " files";
  return s;
}

}
}
