/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Extra utilities to access the file system.
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <Poco/Path.h>
#include <server/common/Array.h>
#include <limits>

namespace sail {

bool hasExtension(Poco::Path p, Array<std::string> extensions);

Poco::Path resolvePath(Poco::Path localPath, const Array<Poco::Path> &candidatesParentPaths);

struct FileScanSettings {
  bool visitDirectories = false;
  bool visitFiles = true;

  /*
   * A maxDepth = 0 means "This path, and nothing else."
   *     The function 'todo' will be called at most once.
   * A maxDepth = 1 means "This path and, if it is a directory, its subpaths."
   */
  int maxDepth = std::numeric_limits<int>::max();
};

void forEveryPath(Poco::Path root, std::function<void(Poco::Path)> todo,
    const FileScanSettings &settings = FileScanSettings(), int currentDepth = 0);

// Recursively traverses a directory and lists all paths for which 'accept' returns true.
Array<Poco::Path> listFilesRecursively(Poco::Path rootPath, std::function<bool(Poco::Path)> accept);

Array<Poco::Path> listFilesRecursivelyByExtension(Poco::Path rootPath, Array<std::string> extensions);

} /* namespace sail */

#endif /* FILESYSTEM_H_ */
