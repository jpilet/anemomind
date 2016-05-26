
#include <server/nautical/tiles/NavTileUploader.h>

#include <algorithm>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <mongo/client/dbclient.h>
#include <mongo/bson/bson-inl.h>
#include <server/common/Optional.h>
#include <server/common/Span.h>
#include <server/common/logging.h>
#include <server/nautical/tiles/NavTileGenerator.h>

using namespace mongo;

#if MONGOCLIENT_VERSION_MAJOR < 1
#define MONGO_QUERY QUERY
namespace mongo { namespace client { void initialize() { } } }
#endif

namespace sail {

using namespace NavCompat;

namespace {
BSONObjBuilder& append(BSONObjBuilder& builder, const char* key,
                       const TimeStamp& value) {
  return builder.appendDate(key,
      value.defined()? Date_t(value.toMilliSecondsSince1970()) : Date_t());
}

BSONObj navToBSON(const Nav& nav) {
  BSONObjBuilder result;

  // GPS is mandatory.
  append(result, "time", nav.time());
  result.append("pos",
                BSON_ARRAY(posToTileX(0, nav.geographicPosition())
                           << posToTileY(0, nav.geographicPosition())));
  result.append("gpsBearing", nav.gpsBearing().degrees());
  result.append("gpsSpeed", nav.gpsSpeed().knots());

  if (nav.hasApparentWind()) {
    result.append("awa", nav.awa().degrees());
    result.append("aws", nav.aws().knots());
  }
  if (nav.hasMagHdg()) {
    result.append("magHdg", nav.magHdg().degrees());
  }
  if (nav.hasWatSpeed()) {
    result.append("watSpeed", nav.watSpeed().knots());
  }
  if (nav.hasExternalTrueWind()) {
    result.append("externalTwa", nav.externalTwa().degrees());
    result.append("externalTws", nav.externalTws().knots());
  }

  Optional<HorizontalMotion<double>> trueWind;
  if (nav.hasTrueWindOverGround()) {
    trueWind = nav.trueWindOverGround();
  } else if (nav.hasApparentWind()) {
    trueWind = nav.estimateTrueWind();
  }

  if (trueWind.defined()) {
    // The following lines assume there is not water current.
    result.append("twdir", calcTwdir(trueWind.get()).degrees());
    result.append("tws", calcTws(trueWind.get()).knots());
    Angle<> twa = calcTwa(trueWind.get(), nav.gpsBearing());
    result.append("twa", twa.degrees());
    result.append("vmg", calcVmg(twa, nav.gpsSpeed()).knots());
  }

  // Old anemobox simulated data.
  if (nav.hasDeviceScreen()) {
    result.append("devicePerf", nav.deviceScreen().perf);
    result.append("deviceTwdir", nav.deviceScreen().twdir);
    result.append("deviceTws", nav.deviceScreen().tws);
  }

  // New anemobox logged data.
  if (nav.hasDeviceVmg()) {
    result.append("deviceVmg", nav.deviceVmg().knots());
  }
  if (nav.hasDeviceTargetVmg()) {
    result.append("deviceTargetVmg", nav.deviceTargetVmg().knots());
  }
  return result.obj();
}

BSONArray navsToBSON(const Array<Nav>& navs) {
  BSONArrayBuilder result;
  for (auto nav: navs) {
    result.append(navToBSON(nav));
  }
  return result.arr();
}

bool safeMongoOps(std::string what,
    DBClientConnection *db, std::function<void(DBClientConnection*)> f) {
  try {
    f(db);
    std::string err = db->getLastError();
    if (err != "") {
      LOG(ERROR) << "error while " << what << ": " << err;
      return false;
    }
  } catch (const DBException &e) {
    LOG(ERROR) << "error while " << what << ": " << e.what();
    return false;
  }
  return true;
}

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

BSONObj locationForSession(const NavDataset& navs) {
  if (getNavSize(navs) == 0) {
    return BSONObj();
  }

  Angle<double> minLat(getNav(navs, 0).geographicPosition().lat());
  Angle<double> minLon(getNav(navs, 0).geographicPosition().lon());
  Angle<double> maxLat(getNav(navs, 0).geographicPosition().lat());
  Angle<double> maxLon(getNav(navs, 0).geographicPosition().lon());
  
  for (auto nav: Range(navs)) {
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
          posToTileX(0, maxPos) - posToTileX(0, minPos),
          posToTileY(0, maxPos) - posToTileY(0, minPos)));
  return location.obj();
}

// Returns average wind speed and average wind direction.
Optional<HorizontalMotion<double>> averageWind(const NavDataset& navs) {
  int num = 0;
  HorizontalMotion<double> sum = HorizontalMotion<double>::zero();
  Velocity<double> sumSpeed = Velocity<double>::knots(0);

  auto marg = Duration<double>::minutes(5.0);
  Span<TimeStamp> validTime(getFirst(navs).time() + marg, getLast(navs).time());

  for (auto nav: Range(navs)) {
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
std::pair<Velocity<double>, int> indexOfStrongestWind(const NavDataset& navs) {
  std::pair<Velocity<double>, int> result(Velocity<double>::knots(0), -1);

  auto marg = Duration<double>::minutes(5.0);
  Span<TimeStamp> validTime(getFirst(navs).time() + marg, getLast(navs).time());

  std::vector<std::pair<Velocity<double>, int>> speedArray;

  int n = getNavSize(navs);
  for (int i = 0; i < n; ++i) {
    const Nav& nav = getNav(navs, i);

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
    NavDataset navs) {
  LOG(INFO) << "Make BSON session";
  LOG(INFO) << navs;

  BSONObjBuilder session;
  session.append("_id", curveId);
  session.append("boat", OID(boatId));
  session.append("trajectoryLength",
      computeTrajectoryLength(navs).nauticalMiles());
  int maxSpeedIndex = findMaxSpeedOverGround(navs);
  if (maxSpeedIndex >= 0) {
    auto nav = getNav(navs, maxSpeedIndex);
    auto speedKnots = nav.gpsSpeed().knots();
    auto timeOfMax = nav.time();
    if (!isFinite(speedKnots)) {
      LOG(WARNING) << "The max speed is not finite for curve '"
          << curveId << "' and boat '" << boatId << "'";
    }
    if (timeOfMax.undefined()) {
      LOG(WARNING) << "The time of max speed is undefined";
    }

    session.append("maxSpeedOverGround", speedKnots);
    append(session, "maxSpeedOverGroundTime", timeOfMax);

  } else {
    LOG(WARNING) << "No max speed found";
  }

  auto startTime = getFirst(navs).time();
  if (startTime.undefined()) {
    LOG(FATAL) << "Start time is undefined";
  }
  append(session, "startTime", startTime);

  auto endTime = getLast(navs).time();
  if (endTime.undefined()) {
    LOG(FATAL) << "End time is undefined";
  }
  append(session, "endTime", endTime);
  session.append("location", locationForSession(navs));

  auto wind = averageWind(navs);
  if (wind.defined()) {
    session.append("avgWindSpeed", calcTws(wind()).knots());
    session.append("avgWindDir", calcTwdir(wind()).degrees());
  } else {
    LOG(WARNING) << "No average wind";
  }

  std::pair<Velocity<double>, int> strongestWind = indexOfStrongestWind(navs);
  if (strongestWind.second >= 0) {
    session.append("strongestWindSpeed", strongestWind.first.knots());
    append(session, "strongestWindTime", getNav(navs, strongestWind.second).time());
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

bool generateAndUploadTiles(std::string boatId,
                            Array<NavDataset> allNavs,
                            const TileGeneratorParameters& params) {
  // Can cause segfault
  // if driver is compiled as C++03, see:
  // https://groups.google.com/forum/#!topic/mongodb-user/-dkp8q9ZEGM
  mongo::client::initialize();

  DBClientConnection db;
  std::string err;
  if (!db.connect(params.dbHost, err)) {
    LOG(ERROR) << "mongoDB connection failed: " << err;
    return false;
  }

  if (params.user.size() > 0) {
    if (!db.auth(params.dbName, params.user, params.passwd, err)) {
      LOG(ERROR) << "mongoDB authentication failed: " << err;
      return false;
    }
  }

  if (params.fullClean) {
    db.remove(params.tileTable(),
               MONGO_QUERY("boat" << OID(boatId)));
    db.remove(params.sessionTable(),
               MONGO_QUERY("boat" << OID(boatId)));
  }

  for (const NavDataset& curve : allNavs) {
    Array<Nav> navs = makeArray(curve);

    std::string curveId = tileCurveId(boatId, curve);

    std::set<TileKey> tiles = tilesForNav(navs, params.maxScale);


    for (auto tileKey : tiles) {
      Array<Array<Nav>> subCurvesInTile = generateTiles(
          tileKey, navs, params.maxNumNavsPerSubCurve);

      if (subCurvesInTile.size() == 0) {
        continue;
      }

      BSONObj tile = makeBsonTile(tileKey, subCurvesInTile, boatId, curveId);

      if (!insertOrUpdateTile(tile, params, &db)) {
        // There is no point to continue if we can't write to the DB.
        return false;
      }
    }
    BSONObj session = makeBsonSession(curveId, boatId, curve);
    if (!insertSession(session, params, &db)) {
      return false;
    }
  }
  return true;
}

}  // namespace sail

