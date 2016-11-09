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

namespace sail {

struct FileTraverseSettings {
  FileTraverseSettings();

  // Whether the 'visitor' function should be called on directories.
  // Even if set to false, the traverse function will still be recursively
  // applied to every directory. TO prevent this, set maxDepth = 0.
  bool visitDirectories;

  // Whether the 'visitor' function should be called on files.
  bool visitFiles;

  // How deep we should go in the file structure. A depth of 0 means
  // that the visitor will at most be called once, on the path passed
  // to the function.
  int maxDepth;
};

void traverseDirectory(
    const Poco::Path &path,
    std::function<void(Poco::Path)> visitor,
    const FileTraverseSettings &settings = FileTraverseSettings(), int currentDepth = 0);

bool hasExtension(Poco::Path p, Array<std::string> extensions);

// Recursively traverses a directory and lists all paths for which 'accept' returns true.
Array<Poco::Path> listFilesRecursively(Poco::Path rootPath, std::function<bool(Poco::Path)> accept);

Array<Poco::Path> listFilesRecursivelyByExtension(Poco::Path rootPath, Array<std::string> extensions);

} /* namespace sail */

#endif /* FILESYSTEM_H_ */
