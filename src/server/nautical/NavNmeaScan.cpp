/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmeaScan.h"
#include "NavNmea.h"
#include <server/common/filesystem.h>
#include <Poco/File.h>

namespace sail {

Array<Nav> scanNmeaFolder(Poco::Path p, Nav::Id boatId,
                          ScreenRecordingSimulator *simulator) {
  { // Initial checks.
    Poco::File file(p);
    if (!file.exists()) {
      return Array<Nav>();
    }

    if (!file.isDirectory()) {
      return Array<Nav>();
    }

    if (boatId.empty()) {
      return Array<Nav>();
    }
  }

  Array<std::string> nmeaExtensions = Array<std::string>::args("txt");
  Array<Poco::Path> files = listFilesRecursivelyByExtension(p, nmeaExtensions);
  int count = files.size();
  Array<ParsedNavs> parsedNavs(count);
  for (int i = 0; i < count; i++) {
    parsedNavs[i] = loadNavsFromNmea(files[i].toString(), boatId);
    if (simulator) {
      simulator->simulate(files[i].toString());
    }
  }
  Array<Nav> result = flattenAndSort(parsedNavs, ParsedNavs::makeGpsWindMask());

  if (simulator) {
    for (Nav& nav : result) {
      ScreenInfo info;
      if (simulator->screenAt(nav.time(), &info)) {
        nav.setDeviceScreen(info);
      }
    }
  }

  return result;
}

} /* namespace sail */
