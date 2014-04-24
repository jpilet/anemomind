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

bool hasExtension(Poco::Path p, Array<std::string> extensions);

// Recursively traverses a directory and lists all paths for which 'accept' returns true.
Array<Poco::Path> listFilesRecursively(Poco::Path rootPath, std::function<bool(Poco::Path)> accept);

Array<Poco::Path> listFilesRecursivelyByExtension(Poco::Path rootPath, Array<std::string> extensions);

} /* namespace sail */

#endif /* FILESYSTEM_H_ */
