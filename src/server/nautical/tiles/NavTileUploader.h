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

  std::string summaryTable() const {
    return dbName + "." + _summaryTable;
  }

  TileGeneratorParameters() {
    dbHost = "localhost";
    maxScale = 17;
    maxNumNavsPerSubCurve = 32;
    _tileTable = "tiles";
    _summaryTable = "summary";
    fullClean = false;
  }
 private:
  std::string _tileTable, _summaryTable;
};

bool generateAndUploadTiles(std::string boatId,
                            Array<Array<Nav>> allNavs,
                            const TileGeneratorParameters& params);

}  // namespace sail

#endif  // NAUTICAL_TILES_NAVTILEUPLOADER_H
