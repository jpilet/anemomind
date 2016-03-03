/*
 *  Created on: 27 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/filesystem.h>
#include <server/common/string.h>
#include <server/common/ArrayBuilder.h>
#include <Poco/DirectoryIterator.h>
#include <assert.h>
#include <server/common/Functional.h>
#include <limits>

namespace sail {

FileTraverseSettings::FileTraverseSettings() :
    visitDirectories(false),
    visitFiles(true),
    maxDepth(std::numeric_limits<int>::max()) {}


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

void traverseDirectory(const Poco::Path &path, std::function<void(Poco::Path)> visitor,
    const FileTraverseSettings &settings, int currentDepth) {
  if (currentDepth > settings.maxDepth) {
    return;
  } else {
    if (path.isDirectory()) {
      if (settings.visitDirectories) {
        visitor(path);
      }
      int deeper = currentDepth + 1;
      Poco::DirectoryIterator end;
        for (Poco::DirectoryIterator it(path); it != end; ++it) {
          traverseDirectory(it->path(), visitor, settings, deeper);
        }
    } else if (settings.visitFiles) {
      visitor(path);
    }
  }
}

Array<Poco::Path> listFilesRecursively(Poco::Path rootPath, std::function<bool(Poco::Path)> accept) {
  FileTraverseSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  ArrayBuilder<Poco::Path> dst;
  traverseDirectory(rootPath, [&](const Poco::Path &path) {
    if (accept(path)) {
      dst.add(path);
    }
  }, settings);
  return dst.get();
}

Array<Poco::Path> listFilesRecursivelyByExtension(Poco::Path rootPath, Array<std::string> extensions) {
  Array<std::string> extensionsLowerCase = toArray(map(extensions, &toLower));
  return listFilesRecursively(rootPath, [&](Poco::Path p) {return hasExtension(p, extensionsLowerCase);});
}

} /* namespace sail */
