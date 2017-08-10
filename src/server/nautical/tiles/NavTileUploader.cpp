
#include <server/nautical/tiles/NavTileUploader.h>

#include <algorithm>
#include <boost/noncopyable.hpp>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <server/common/Optional.h>
#include <server/common/Span.h>
#include <server/common/logging.h>
#include <server/nautical/MaxSpeed.h>
#include <server/nautical/tiles/MongoUtils.h>
#include <server/nautical/tiles/NavTileGenerator.h>


/*
#if MONGOCLIENT_VERSION_MAJOR < 1
#define MONGO_QUERY QUERY
namespace mongo { namespace client { void initialize() { } } }
#endif
*/

namespace sail {

using namespace NavCompat;
using std::vector;
using std::map;

namespace {
void navToBSON(const Nav& nav, bson_t* result) {

  // GPS is mandatory.
  append(result, "time", nav.time());

  double posXY[2] = {
      posToTileX(0, nav.geographicPosition()),
      posToTileY(0, nav.geographicPosition())
  };
  bsonAppendElements(result, "pos", posXY, posXY + 2);

  bsonAppend(result, "gpsBearing", nav.gpsBearing().degrees());
  bsonAppend(result, "gpsSpeed", nav.gpsSpeed().knots());

  if (nav.hasApparentWind()) {
    bsonAppend(result, "awa", nav.awa().degrees());
    bsonAppend(result, "aws", nav.aws().knots());
  }
  if (nav.hasMagHdg()) {
    bsonAppend(result, "magHdg", nav.magHdg().degrees());
  }
  if (nav.hasWatSpeed()) {
    bsonAppend(result, "watSpeed", nav.watSpeed().knots());
  }
  if (nav.hasExternalTrueWind()) {
    bsonAppend(result, "externalTwa", nav.externalTwa().degrees());
    bsonAppend(result, "externalTws", nav.externalTws().knots());
  }

  Optional<HorizontalMotion<double>> trueWind;
  if (nav.hasTrueWindOverGround()) {
    trueWind = nav.trueWindOverGround();
  } else if (nav.hasApparentWind()) {
    trueWind = nav.estimateTrueWind();
  }

  if (trueWind.defined()) {
    // The following lines assume there is not water current.
    bsonAppend(result, "twdir", calcTwdir(trueWind.get()).degrees());
    bsonAppend(result, "tws", calcTws(trueWind.get()).knots());
    Angle<> twa = calcTwa(trueWind.get(), nav.gpsBearing());
    bsonAppend(result, "twa", twa.degrees());
    bsonAppend(result, "vmg", calcVmg(twa, nav.gpsSpeed()).knots());
  }

  // Old anemobox simulated data.
  if (nav.hasDeviceScreen()) {
    bsonAppend(result, "devicePerf", nav.deviceScreen().perf);
    bsonAppend(result, "deviceTwdir", nav.deviceScreen().twdir);
    bsonAppend(result, "deviceTws", nav.deviceScreen().tws);
  }

  // New anemobox logged data.
  if (nav.hasDeviceVmg()) {
    bsonAppend(result, "deviceVmg", nav.deviceVmg().knots());
  }
  if (nav.hasDeviceTargetVmg()) {
    bsonAppend(result, "deviceTargetVmg", nav.deviceTargetVmg().knots());
  }
}

BSONArray navsToBSON(const Array<Nav>& navs) {
  BSONArrayBuilder result;
  for (auto nav: navs) {
    result.append(navToBSON(nav));
  }
  return result.arr();
}

class TileInserter : public BulkInserter {
 public:
  TileInserter(const TileGeneratorParameters& params, DBClientConnection* db)
    : BulkInserter(params.tileTable(), 1000, db), _params(params) { }

  bool insert(const BSONObj& obj) {
    if (!_params.fullClean) {
      safeMongoOps("cleaning old tiles",
          db(), [=](DBClientConnection *db) {
        db->remove(_params.tileTable(),
                   MONGO_QUERY("key" << obj["key"]
                         << "boat" << obj["boat"]
                         << "startTime" << GTE << obj["startTime"]
                         << "endTime" << LTE << obj["endTime"]));
      });
    }
    return BulkInserter::insert(obj);
  }
 private:
  const TileGeneratorParameters& _params;
};

bool insertOrUpdateTile(const BSONObj& obj,
    const TileGeneratorParameters& params,
                        DBClientConnection* db) {
  if (!params.fullClean) {
    safeMongoOps("cleaning old tiles",
        db, [=](DBClientConnection *db) {
      db->remove(params.tileTable(),
                 MONGO_QUERY("key" << obj["key"]
                       << "boat" << obj["boat"]
                       << "startTime" << GTE << obj["startTime"]
                       << "endTime" << LTE << obj["endTime"]));
    });
  }
  return safeMongoOps("inserting a tile in mongoDB",
      db, [=](DBClientConnection *db) {
    db->insert(params.tileTable(), obj);
  });
}

bool insertSession(const BSONObj &obj,
  const TileGeneratorParameters& params,
  DBClientConnection *db) {
  return safeMongoOps("updating a session", db,
    [=](DBClientConnection *db) {
    db->update(params.sessionTable(),// <-- The collection
        MONGO_QUERY("_id" << obj["_id"]),  // <-- what to update
        obj,                         // <-- the new data
        true,                        // <-- upsert
        false);                      // <-- multi
  });
}

template <typename T>
Angle<T> average(const Angle<T>& a, const Angle<T>& b) {
  HorizontalMotion<T> motion =
    HorizontalMotion<T>::polar(Velocity<T>::knots(1), a);
  motion = motion + HorizontalMotion<T>::polar(Velocity<T>::knots(1), b);
  return motion.angle();
}

BSONObj locationForSession(const Array<Nav>& navs) {
  if (navs.size() == 0) {
    return BSONObj();
  }

  Angle<double> minLat(navs[0].geographicPosition().lat());
  Angle<double> minLon(navs[0].geographicPosition().lon());
  Angle<double> maxLat(navs[0].geographicPosition().lat());
  Angle<double> maxLon(navs[0].geographicPosition().lon());
  
  for (auto nav: navs) {
    minLat = std::min(minLat, nav.geographicPosition().lat());
    maxLat = std::max(maxLat, nav.geographicPosition().lat());
    minLon = std::min(minLon, nav.geographicPosition().lon());
    maxLon = std::max(maxLon, nav.geographicPosition().lon());
  }

  GeographicPosition<double> center(
      average(minLon, maxLon), average(minLat, maxLat));

  GeographicPosition<double> minPos(minLon, minLat);
  GeographicPosition<double> maxPos(maxLon, maxLat);

  BSONObjBuilder location;
  location.append("x", posToTileX(0, center));
  location.append("y", posToTileY(0, center));
  location.append("scale", 2 * std::max(
          fabs(posToTileX(0, maxPos) - posToTileX(0, minPos)),
          fabs(posToTileY(0, maxPos) - posToTileY(0, minPos))));
  return location.obj();
}

// Returns average wind speed and average wind direction.
Optional<HorizontalMotion<double>> averageWind(const Array<Nav>& navs) {
  int num = 0;
  HorizontalMotion<double> sum = HorizontalMotion<double>::zero();
  Velocity<double> sumSpeed = Velocity<double>::knots(0);

  auto marg = Duration<double>::minutes(5.0);
  Span<TimeStamp> validTime(navs.first().time() + marg,
                            navs.last().time());

  for (auto nav: navs) {
    if (!validTime.contains(nav.time())) {
      continue;
    }

    // We prefer Anemomind-calibrated wind over external instrument wind.
    if (nav.hasTrueWindOverGround()) {
      auto tws = calcTws(nav.trueWindOverGround());
      if (tws.knots() > 0) {
        sumSpeed += tws;
        num++;
        sum += nav.trueWindOverGround().scaled(1.0 / tws.knots());
      }
    } else if (nav.hasExternalTrueWind()) {
      num++;
      sum += windMotionFromTwdirAndTws(
          nav.externalTwdir(), Velocity<double>::knots(1));
      sumSpeed += nav.externalTws();
    } else if (nav.hasApparentWind()) {
      HorizontalMotion<double> trueWind = nav.estimateTrueWind();
      auto tws = calcTws(trueWind);
      if (tws.knots() > 0) {
        sumSpeed += tws;
        num++;
        sum += trueWind.scaled(1.0 / tws.knots());
      }
    }
  }

  if (num > 0) {
    return Optional<HorizontalMotion<double>>(
        HorizontalMotion<double>::polar(sumSpeed.scaled(1.0 / num),
                                        sum.angle()));
  }
  return Optional<HorizontalMotion<double>>();
}

// Returns the strongest wind and the corresponding nav index.
// If no wind information is present, the returned index is -1.
std::pair<Velocity<double>, int> indexOfStrongestWind(const Array<Nav>& navs) {
  std::pair<Velocity<double>, int> result(Velocity<double>::knots(0), -1);

  auto marg = Duration<double>::minutes(5.0);
  Span<TimeStamp> validTime(navs.first().time() + marg,
                            navs.last().time());

  std::vector<std::pair<Velocity<double>, int>> speedArray;

  for (int i = 0; i < navs.size(); ++i) {
    const Nav& nav = navs[i];

    // If the boat is not moving, the strongest wind is not so interesting.
    // The following if avoids most outliers.
    if (validTime.contains(nav.time())
        && nav.gpsSpeed() > Velocity<double>::knots(1)) {

      if (nav.hasTrueWindOverGround()) {
        speedArray.push_back(make_pair(calcTws(nav.trueWindOverGround()), i));
      } else if (nav.hasExternalTrueWind()) {
        speedArray.push_back(make_pair(nav.externalTws(), i));
      } else if (nav.hasApparentWind()) {
        speedArray.push_back(make_pair(calcTws(nav.estimateTrueWind()), i));
      }
    }
  }

  if (speedArray.size() > 0) {
    // Take the 99% quantile to eliminate outliers.
    auto it = speedArray.begin() + speedArray.size() * 99 / 100;
    nth_element(speedArray.begin(), it, speedArray.end());
    return *it;
  }
  return std::pair<Velocity<double>, int>(Velocity<double>::knots(0), -1);
}

BSONObj makeBsonSession(
    const std::string &curveId,
    const std::string &boatId,
    NavDataset navs,
    const Array<Nav>& navArray,
    DOM::Node *li) {
  BSONObjBuilder session;
  session.append("_id", curveId);
  session.append("boat", OID(boatId));
  session.append("trajectoryLength",
      computeTrajectoryLength(navs).nauticalMiles());

  Optional<TimedValue<Velocity<double>>> maxSpeed =
    computeMaxSpeedOverPeriod(navs);

  if (maxSpeed.defined()) {
    DOM::addSubTextNode(li, "p",
        stringFormat("BSON session max speed: %.3g knots",
            maxSpeed.get().value.knots()));
    session.append("maxSpeedOverGround", maxSpeed.get().value.knots());
    append(session, "maxSpeedOverGroundTime", maxSpeed.get().time);
  } else {
    LOG(WARNING) << "The max speed is not defined for curve '"
        << curveId << "' and boat '" << boatId << "'";
  }

  auto startTime = navArray[0].time();
  if (startTime.undefined()) {
    LOG(FATAL) << "Start time is undefined";
  }
  append(session, "startTime", startTime);

  auto endTime = navArray.last().time();
  if (endTime.undefined()) {
    LOG(FATAL) << "End time is undefined";
  }
  append(session, "endTime", endTime);
  session.append("location", locationForSession(navArray));

  auto wind = averageWind(navArray);
  if (wind.defined()) {
    session.append("avgWindSpeed", calcTws(wind()).knots());
    session.append("avgWindDir", calcTwdir(wind()).degrees());
  } else {
    LOG(WARNING) << "No average wind";
  }

  std::pair<Velocity<double>, int> strongestWind = indexOfStrongestWind(navArray);
  if (strongestWind.second >= 0) {
    session.append("strongestWindSpeed", strongestWind.first.knots());
    append(session, "strongestWindTime", navArray[strongestWind.second].time());
  } else {
    LOG(WARNING) << "No strongest wind";
  }

  return session.obj();
}

BSONObj makeBsonTile(const TileKey& tileKey,
                     const Array<Array<Nav>>& subCurvesInTile,
                     const std::string& boatId,
                     const std::string& curveId) {
  BSONObjBuilder tile;
  tile.genOID();
  tile.append("key", tileKey.stringKey());
  tile.append("boat", OID(boatId));
  append(tile, "startTime", subCurvesInTile.first().first().time());
  append(tile, "endTime", subCurvesInTile.last().last().time());
  append(tile, "created", TimeStamp::now());

  std::vector<BSONObj> curves;
  for (auto subCurve: subCurvesInTile) {
    BSONObjBuilder subCurveBuilder;

    subCurveBuilder
      .append("curveId", curveId)
      .append("points", navsToBSON(subCurve));

    curves.push_back(subCurveBuilder.obj());
  }
  tile.append("curves", curves);
  return tile.obj();
}

}  // namespace


void removeBoatWithId(
    mongoc_database_t* db, const std::string& tableName,
    const std::string& boatId) {
  auto coll = UNIQUE_MONGO_PTR(
      mongoc_collection,
      mongoc_database_get_collection(
          db, tableName.c_str()));
  auto oid = makeOid(boatId);
  withTemporaryBsonDocument([&](bson_t* query) {
    BSON_APPEND_OID(query, "boat", &oid);
    mongoc_collection_remove(
        coll.get(),
        MONGOC_REMOVE_NONE,
        query, nullptr, nullptr);
  });
}

bool generateAndUploadTiles(std::string boatId,
                            Array<NavDataset> allNavs,
                            mongoc_database_t* db,
                            const TileGeneratorParameters& params) {
  if (params.fullClean) {
    removeBoatWithId(db, params.tileTable(), boatId);
    removeBoatWithId(db, params.sessionTable(), boatId);
  }

  TileInserter inserter(params, db);
  DOM::Node d2 = params.log; // Workaround
  auto page = DOM::linkToSubPage(&d2, "generateAndUploadTiles");
  auto ul = DOM::makeSubNode(&page, "ul");
  for (const NavDataset& curve : allNavs) {
    auto li = DOM::makeSubNode(&ul, "li");

    Array<Nav> navs = makeArray(curve);

    std::string curveId = tileCurveId(boatId, curve);

    DOM::addSubTextNode(&li, "p",
        stringFormat("Curve with id %s and %d navs", curveId.c_str(), navs.size()));

    map<TileKey, vector<int>> tiles = tilesForNav(navs, params.maxScale);

    for (auto it : tiles) {
      const TileKey& tileKey = it.first;
      Array<Array<Nav>> subCurvesInTile = generateTiles(
          tileKey,
          navs,
          it.second, // the vector of nav indices
          params.maxNumNavsPerSubCurve, params.curveCutThreshold);

      if (subCurvesInTile.size() == 0) {
        continue;
      }

      auto tile = makeBsonTile(tileKey, subCurvesInTile, boatId, curveId);

      if (!inserter.insert(tile)) {
        // There is no point to continue if we can't write to the DB.
        return false;
      }
    }
    auto session = makeBsonSession(curveId, boatId, curve, navs, &li);
    if (!insertSession(session, params, db)) {
      return false;
    }
  }

  return inserter.finish();
}

}  // namespace sail

