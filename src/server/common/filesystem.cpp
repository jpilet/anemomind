/*
 *  Created on: 27 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "filesystem.h"
#include <server/common/string.h>
#include <Poco/DirectoryIterator.h>
#include <assert.h>
#include <server/common/Functional.h>

namespace sail {

bool hasExtension(Poco::Path p, Array<std::string> extensions) {
  assert(p.isFile());
  std::string ext = toLower(p.getExtension());
  int count = extensions.size();
  for (int i = 0; i < count; i++) {
    if (ext == toLower(extensions[i])) {
      return true;
    }
  }
  return false;
}

namespace {
  void listFilesRecursivelySub(Poco::Path path, std::function<bool(Poco::Path it)> accept,
    std::vector<Poco::Path> *paths) {
    Poco::DirectoryIterator end;
    for (Poco::DirectoryIterator it(path); it != end; ++it) {
      if (accept(it->path())) {
        paths->push_back(it->path());
      }
      if (it->isDirectory()) {
        listFilesRecursivelySub(it->path(), accept, paths);
      }
    }
  }
}


// See http://www.codeproject.com/Articles/254346/Learning-Poco-List-directories-recursively
Array<Poco::Path> listFilesRecursively(Poco::Path rootPath, std::function<bool(Poco::Path)> accept) {
  std::vector<Poco::Path> dst;
  listFilesRecursivelySub(rootPath, accept, &dst);
  return Array<Poco::Path>::referToVector(dst).dup();
}

Array<Poco::Path> listFilesRecursivelyByExtension(Poco::Path rootPath, Array<std::string> extensions) {
  Array<std::string> extensionsLowerCase = toArray(map(&toLower, extensions));
  return listFilesRecursively(rootPath, [&](Poco::Path p) {return hasExtension(p, extensionsLowerCase);});
}

} /* namespace sail */
