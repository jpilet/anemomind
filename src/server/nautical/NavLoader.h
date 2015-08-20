#ifndef NAUTICAL_NAVLOADER_H
#define NAUTICAL_NAVLOADER_H

#include <string>
#include <server/nautical/NavNmea.h>

namespace sail {

// Detect the file format according to the extension and tries to load it.
ParsedNavs loadNavsFromFile(std::string file, Nav::Id boatId);

}  // namespace sail

#endif // NAUTICAL_NAVLOADER_H
