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

Poco::Path resolvePath(Poco::Path localPath, const Array<Poco::Path> &candidateParentPaths) {
  if (Poco::File(localPath).exists()) {
    return localPath;
  } else {
    for (auto c: candidateParentPaths) {
      c.append(localPath);
      if (Poco::File(c).exists()) {
        return c;
      }
    }
  }
  return Poco::Path();
}

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


void forEveryPath(Poco::Path path, std::function<void(Poco::Path)> todo,
    const FileScanSettings &settings, int currentDepth) {
  auto currentFile = Poco::File(path);
  if (currentFile.isDirectory()) {
    if (settings.visitDirectories) {
      todo(path);
    }
    if (currentDepth < settings.maxDepth) {
      Poco::DirectoryIterator end;
      for (Poco::DirectoryIterator it(path); it != end; ++it) {
        forEveryPath(it->path(), todo, settings, currentDepth + 1);
      }
    }
  } else if (currentFile.isFile() && settings.visitFiles) {
    todo(path);
  }
}

// See http://www.codeproject.com/Articles/254346/Learning-Poco-List-directories-recursively
Array<Poco::Path> listFilesRecursively(Poco::Path rootPath, std::function<bool(Poco::Path)> accept) {
  std::vector<Poco::Path> dst;
  FileScanSettings settings;
  settings.visitDirectories = false;
  settings.visitFiles = true;
  forEveryPath(rootPath, [&](const Poco::Path &p) {
    if (accept(p)) {
      dst.push_back(p);
    }
  }, settings);
  return Array<Poco::Path>::referToVector(dst).dup();
}

Array<Poco::Path> listFilesRecursivelyByExtension(Poco::Path rootPath, Array<std::string> extensions) {
  Array<std::string> extensionsLowerCase = toArray(map(extensions, &toLower));
  return listFilesRecursively(rootPath, [&](Poco::Path p) {return hasExtension(p, extensionsLowerCase);});
}

} /* namespace sail */
