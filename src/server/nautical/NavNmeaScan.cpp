/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmeaScan.h"
#include "NavNmea.h"
#include <Poco/File.h>
#include <server/common/filesystem.h>
#include <server/common/logging.h>

namespace sail {

Array<Nav> scanNmeaFolder(Poco::Path p, Nav::Id boatId,
                          ScreenRecordingSimulator *simulator, ParsedNavs::FieldMask mask) {
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

    LOG(INFO) << "Parsed " << files[i].toString();

    if (simulator) {
      simulator->simulate(files[i].toString());
    }
  }
  Array<Nav> result = flattenAndSort(parsedNavs, mask);

  if (simulator) {
    for (Nav& nav : result) {
      ScreenInfo info;
      if (simulator->screenAt(nav.time(), &info)) {
        auto delta = fabs(nav.time() - info.time);
        if (delta > Duration<>::seconds(4)) {
          LOG(WARNING) << "Time problem while matching simulated data to nav: "
            << delta.str() << " of time difference, at "
            << nav.time().toString();
        } else {
          nav.setDeviceScreen(info);
        }
      }
    }
  }

  return result;
}

Array<Nav> scanNmeaFolder2(Poco::Path p, Nav::Id boatId,
                          ParsedNavs::FieldMask mask) {
  return scanNmeaFolder(p, boatId, nullptr, mask);
}


} /* namespace sail */
