/*
 *  Created on: 27 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "filesystem.h"
#include <server/common/string.h>
#include <Poco/DirectoryIterator.h>
#include <assert.h>

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

bool isNmeaFilePath(Poco::Path p) {
  const int extcount = 1;
  std::string extensions[extcount] = {"txt"};
  if (p.isFile()) {
    return hasExtension(p, Array<std::string>(extcount, extensions));
  } else {
    return false;
  }
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

} /* namespace sail */
