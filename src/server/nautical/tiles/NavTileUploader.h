#ifndef NAUTICAL_TILES_NAVTILEUPLOADER_H
#define NAUTICAL_TILES_NAVTILEUPLOADER_H

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <string>
#include <server/common/Array.h>
#include <server/nautical/NavCompatibility.h>
#include <server/nautical/tiles/MongoUtils.h>
#include <server/common/DOMUtils.h>

namespace sail {

struct TileGeneratorParameters  {
  DOM::Node log;
  int maxScale;
  int maxNumNavsPerSubCurve;
  bool fullClean;
  Duration<> curveCutThreshold;
  std::string mongoUri = MongoDBConnection::defaultMongoUri();

  std::shared_ptr<mongoc_uri_t> uri() const {
    return SHARED_MONGO_PTR(
        mongoc_uri,
        mongoc_uri_new(mongoUri.c_str()));
  }

  std::string dbName() const {
    return mongoc_uri_get_database(uri().get());
  }

  MongoTableName tileTable() const {
    return MongoTableName(dbName(), _tileTable);
  }

  MongoTableName sessionTable() const {
    return MongoTableName(dbName(), _sessionTable);
  }

  TileGeneratorParameters() {
    maxScale = 17;
    maxNumNavsPerSubCurve = 32;
    _tileTable = "tiles";
    _sessionTable = "sailingsessions";
    fullClean = false;
    curveCutThreshold = Duration<>::minutes(1);
  }
 private:
  std::string _tileTable, _sessionTable;
};

bool generateAndUploadTiles(std::string boatId,
                            Array<NavDataset> allNavs,
                            const std::shared_ptr<mongoc_database_t>& db,
                            const TileGeneratorParameters& params);

}  // namespace sail

#endif  // NAUTICAL_TILES_NAVTILEUPLOADER_H
