/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NavNmeaScan.h"
#include "NavNmea.h"
#include <server/common/filesystem.h>
#include <Poco/File.h>

namespace sail {

Array<Nav> scanNmeaFolder(Poco::Path p, Nav::Id boatId) {
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
  }
  return flattenAndSort(parsedNavs, ParsedNavs::makeGpsWindMask());
}

} /* namespace sail */
