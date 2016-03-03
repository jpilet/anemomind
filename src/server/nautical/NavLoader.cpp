
#include <server/nautical/NavLoader.h>

#include <Poco/Path.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/NavNmea.h>
#include <server/nautical/NavCsv.h>

namespace sail {

ParsedNavs loadNavsFromFile(std::string file, Nav::Id boatId) {
  std::string ext = toLower(Poco::Path(file).getExtension());

  if (ext == "txt") {
    return loadNavsFromNmea(file, boatId);
  } else if (ext == "csv") {
    return ParsedNavs(NavCsv::parse(file));
  } else {
    LOG(ERROR) << file << ": unknown extension.";
    return ParsedNavs();
  }
}

}  // namespace sail
