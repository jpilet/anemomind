#ifndef NAUTICAL_TILES_NAVTILEUPLOADER_H
#define NAUTICAL_TILES_NAVTILEUPLOADER_H

#include <string>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>

namespace sail {

struct TileGeneratorParameters {
  std::string dbHost;
  int maxScale;
  int maxNumNavsPerSubCurve;
  std::string dbName;
  bool fullClean;

  std::string tileTable() const {
    return dbName + "." + _tileTable;
  }

  std::string sessionTable() const {
    return dbName + "." + _sessionTable;
  }

  TileGeneratorParameters() {
    dbName = "anemomind-dev";
    dbHost = "localhost";
    maxScale = 17;
    maxNumNavsPerSubCurve = 32;
    _tileTable = "tiles";
    _sessionTable = "sailingsessions";
    fullClean = false;
  }
 private:
  std::string _tileTable, _sessionTable;
};

bool generateAndUploadTiles(std::string boatId,
                            Array<NavCollection> allNavs,
                            const TileGeneratorParameters& params);

}  // namespace sail

#endif  // NAUTICAL_TILES_NAVTILEUPLOADER_H
