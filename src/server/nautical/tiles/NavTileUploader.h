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
  std::string tileTable;
  bool fullClean;

  TileGeneratorParameters() {
    dbHost = "localhost";
    maxScale = 17;
    maxNumNavsPerSubCurve = 32;
    tileTable = "anemomind-dev.tiles";
    fullClean = false;
  }
};

bool generateAndUploadTiles(std::string boatId,
                            Array<Array<Nav>> allNavs,
                            const TileGeneratorParameters& params);

}  // namespace sail

#endif  // NAUTICAL_TILES_NAVTILEUPLOADER_H
