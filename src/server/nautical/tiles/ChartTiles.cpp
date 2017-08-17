#include <server/nautical/tiles/ChartTiles.h>

#include <functional>
#include <device/anemobox/Dispatcher.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/tiles/MongoUtils.h>
#include <string>
#include <server/common/logging.h>

using std::map;
using std::shared_ptr;
using std::string;

namespace sail {

// A tile at zoom level z spans 2^z seconds
// Tile number t for zoom z contains samples between
// t * 2^z (included)
// and (t + 1) * 2^z (not included).
int64_t tileAt(TimeStamp time, int zoom) {
  return time.toSecondsSince1970() >> zoom;
}

TimeStamp tileBeginTime(int64_t tile, int zoom) {
  return TimeStamp::fromMilliSecondsSince1970((tile << zoom) * 1000);
}

TimeStamp tileEndTime(int64_t tile, int zoom) {
  return tileBeginTime(tile + 1, zoom);
}


namespace {

template <typename T>
void downSampleData(int64_t tileno, int zoom,
                    TypedDispatchData<T>* data,
                    const ChartTileSettings& settings,
                    const map<int64_t, ChartTile<T>>* prevZoomTiles,
                    ChartTile<T> *result) {
  Duration<> tileSpan = Duration<>::seconds(1 << zoom);
  Duration<> samplingPeriod = tileSpan.scaled(1.0 / settings.samplesPerTile);
  TimeStamp end = tileEndTime(tileno, zoom);

  result->zoom = zoom;
  result->tileno = tileno;

  if (prevZoomTiles) {
    // build from higher resolution tiles
    
    if (prevZoomTiles->find(tileno * 2) == prevZoomTiles->end()
        && prevZoomTiles->find(tileno * 2 + 1) == prevZoomTiles->end()) {
      return;
    }

    bool empty = true;
    for (int i = 0; i < 2; ++i) {
      int subtile = tileno * 2 + i;
      auto it = prevZoomTiles->find(subtile);
      if (it != prevZoomTiles->end()) {
        const TimedSampleCollection<Statistics<T>>& samples = it->second.samples;
        assert(samples.size() == settings.samplesPerTile);
        for (int s = 0; s < samples.size(); s += 2) {
          // Combine two samples into a single one
          assert(result->samples.size() == 0
                 || result->samples[result->samples.size() - 1].time < samples[s].time);
          result->samples.append(
              samples[s].time, samples[s + 0].value + samples[s + 1].value);
        }
        empty = false;
      } else {
        TimeStamp subtileStartTime = tileBeginTime(subtile, zoom - 1);
        TimeStamp subtileEndTime = tileEndTime(subtile, zoom - 1);
        Duration<> subtileSpan = tileSpan.scaled(.5);
        for (int j = 0; j < settings.samplesPerTile / 2; ++j) {
          TimeStamp time = subtileStartTime + subtileSpan.scaled(
              double(j) / double(settings.samplesPerTile));

          assert(time >= subtileStartTime);
          assert(time < subtileEndTime);
          assert(result->samples.size() == 0
                 || result->samples[result->samples.size() - 1].time < time);
          result->samples.append(time, Statistics<T>());
        }
      }
    }
    if (empty) {
      result->samples.clear();
    }
  } else {
    // build from data
    const std::deque<TimedValue<T>>& values = data->dispatcher()->values().samples();
    auto firstOfTile = std::lower_bound(values.begin(), values.end(),
                                        tileBeginTime(tileno, zoom));
    auto firstAfterTile = std::lower_bound(values.begin(), values.end(),
                                           tileEndTime(tileno, zoom));
    if (firstOfTile == firstAfterTile) {
      // nothing within the range of this tile.
      return;
    }

    int totalCount = 0;
    for (TimeStamp time = tileBeginTime(tileno, zoom);
         time < end; time += samplingPeriod) {
      // Compute stats over all samples within [time, time + samplingPeriod[
      auto first = std::lower_bound(values.begin(), values.end(), time);
      auto last = std::lower_bound(values.begin(), values.end(), time + samplingPeriod);
      Statistics<T> stats;
      for (auto it = first; it != last; ++it) {
        assert(time <= it->time);
        assert(it->time < (time + samplingPeriod));
        stats.add(it->value);
        totalCount++;
      }
      assert(result->samples.size() == 0
             || result->samples[result->samples.size() - 1].time < time);
      result->samples.append(time, stats);
    }
    assert(result->samples.size() == settings.samplesPerTile);
    assert(totalCount > 0);
  }
}

template<class T>
bool chartTileToBson(const ChartTile<T> tile,
                     const std::string& boatId,
                     const DispatchData* data,
                     std::shared_ptr<bson_t>* dst) {
  if (tile.samples.size() == 0) {
    return false;
  }

  auto result = SHARED_MONGO_PTR(bson, bson_new());
  // The key is function of:
  // boatId, zoom, tileno, code, source.
  // The order matters, because mongodb indexes first on boatId, then zoom,
  // the tileno, etc. As a result, mongo will used the index _id to resolve
  // the following query:
  // db.chartTiles.find({
  //   "_id.boat" : "xxx",
  //   "_id.zoom" : 8,
  //   "_id.tileno" : 256});
  // and it will return all codes + all sources available for this boat, zoom,
  // and tileno.
  {
    BsonSubDocument key(result.get(), "_id");
    auto oid = makeOid(boatId);
    BSON_APPEND_OID(&key, "boat", &oid);
    BSON_APPEND_INT32(&key, "zoom", tile.zoom);
    BSON_APPEND_INT64(&key, "tileno", (long long) tile.tileno);
    BSON_APPEND_UTF8(&key, "what", data->wordIdentifier().c_str());
    BSON_APPEND_UTF8(&key, "source", data->source().c_str());
  }

  StatArrays arrays;

  for (const TimedValue<Statistics<T>>& stats : tile.samples) {
    stats.value.appendToArrays(&arrays);
  }

  bsonAppendCollection(result.get(), "mean", arrays.mean);
  bsonAppendCollection(result.get(), "min", arrays.min);
  bsonAppendCollection(result.get(), "max", arrays.max);
  bsonAppendCollection(result.get(), "count", arrays.count);

  *dst = result;

  return true;
}

template<class T>
bool uploadChartTile(const ChartTile<T> tile,
                     const DispatchData* data,
                     const std::string& boatId,
                     const ChartTileSettings& settings,
                     BulkInserter *inserter) {
  std::shared_ptr<bson_t> obj;
  if (!chartTileToBson(tile, boatId, data, &obj)) {
    // Uploading an empty tile does not make sense.
    // But it is not an error.
    return true;
  }

  return inserter->insert(obj);
}

class UploadChartTilesVisitor : public DispatchDataVisitor {
 public:
  UploadChartTilesVisitor(const std::string& boatId,
                          const ChartTileSettings& settings,
                          BulkInserter *inserter)
    : _boatId(boatId), _settings(settings), _inserter(inserter), _result(true) { }

  template<class T>
  void makeTiles(TypedDispatchData<T> *data) {
    const TimedSampleCollection<T>& values = data->dispatcher()->values();
    if (values.size() == 0) {
      // nothing to do on empty collections.
      return;
    }
    if (!_result) {
      // the process has already failed.
      return;
    }

    TimeStamp firstTime = values.first().time;
    TimeStamp lastTime = values.last().time;

    map<int64_t, ChartTile<T>> prevZoomTiles;

    for (int zoom = _settings.lowestZoomLevel; zoom <= _settings.highestZoomLevel; zoom++) {
      int64_t firstTile = tileAt(firstTime, zoom);
      int64_t lastTile = tileAt(lastTime, zoom);

      map<int64_t, ChartTile<T>> tiles;

      for (int64_t tileno = firstTile; tileno <= lastTile; ++tileno) {
        ChartTile<T> tile;
        downSampleData(tileno, zoom, data, _settings,
                       (zoom == _settings.lowestZoomLevel ?
                           nullptr : &prevZoomTiles),
                       &tile);
        if (!tile.empty()) {
          tiles[tileno] = tile;
          if (!uploadChartTile(tiles[tileno], data, _boatId, _settings, _inserter)) {
            _result = false;
            return;
          }
        }
      }

      // Keep previous zoom tiles so that downSampleData can use them
      // to produce the next zoom level.
      prevZoomTiles.swap(tiles);
    }
  }

  virtual void run(DispatchAngleData *angle) { makeTiles(angle); }
  virtual void run(DispatchVelocityData *velocity) { makeTiles(velocity); }
  virtual void run(DispatchLengthData *length) { makeTiles(length); }
  virtual void run(DispatchGeoPosData *pos) { /* nothing for pos */ }
  virtual void run(DispatchTimeStampData *timestamp) { /* nothing */ }
  virtual void run(DispatchAbsoluteOrientationData *orient) { /* TODO */ }
  virtual void run(DispatchBinaryEdge *binary) { /* nothing */ }

  bool result() const { return _result; }

 private:
  const std::string& _boatId;
  const ChartTileSettings& _settings;
  BulkInserter *_inserter;
  bool _result;
};

class GetMinMaxTime : public DispatchDataVisitor {
 public:
  GetMinMaxTime() : valid(false) { }

  template<typename T>
  void getFirstLast(TypedDispatchData<T> *tdd) {
    const TimedSampleCollection<T> values = tdd->dispatcher()->values();
    if (values.size() > 0) {
      first = values[0].time;
      last = values.lastTimeStamp();
      valid = true;
    }
  }
  TimeStamp first, last;
  bool valid;

  virtual void run(DispatchAngleData *angle) { getFirstLast(angle); }
  virtual void run(DispatchVelocityData *velocity) { getFirstLast(velocity); }
  virtual void run(DispatchLengthData *length) { getFirstLast(length); }
  virtual void run(DispatchGeoPosData *pos) { /* nothing for pos */ }
  virtual void run(DispatchTimeStampData *timestamp) { /* nothing */ }
  virtual void run(DispatchAbsoluteOrientationData *orient) { }
  virtual void run(DispatchBinaryEdge *orient) { }
};

bool uploadChartTiles(DispatchData* data,
                      const std::string& boatId,
                      const ChartTileSettings& settings,
                      BulkInserter *db) {
  UploadChartTilesVisitor visitor(boatId, settings, db);
  data->visit(&visitor);
  return visitor.result();
}

}  // namespace

bool uploadChartTiles(const NavDataset& data,
                      const std::string& boatId,
                      const ChartTileSettings& settings,
                      const std::shared_ptr<mongoc_database_t>& db) {
  const map<DataCode, map<string, shared_ptr<DispatchData>>> &allSources =
    data.dispatcher()->allSources();

  auto collection = UNIQUE_MONGO_PTR(
      mongoc_collection,
      mongoc_database_get_collection(
          db.get(),
          settings.table().localName().c_str()));

  //MONGO_QUERY("_id.boat" << OID(boatId)),


  auto oid = makeOid(boatId);
  {
    WrapBson selector;
    {
      BsonSubDocument id(&selector, "_id");
      BSON_APPEND_OID(&id, "boat", &oid);
    }
    auto concern = mongoWriteConcernForLevel(MONGOC_WRITE_CONCERN_W_DEFAULT);
    bson_error_t error;
    if (!mongoc_collection_remove(
        collection.get(),
        MONGOC_REMOVE_NONE,
        &selector,
        concern.get(),
        &error)) {
      LOG(ERROR) << "Failed to execute remove old chart "
          "tiles for boat, but we will continue: "
          << bsonErrorToString(error);
    }
  }

  auto coll = SHARED_MONGO_PTR(
      mongoc_collection,
      mongoc_database_get_collection(
          db.get(),
          settings.table().localName().c_str()));
  if (!coll) {
    LOG(ERROR) << "Failed to collection";
    return false;
  }
  BulkInserter inserter(coll, 1000);

  for (auto channel : allSources) {
    for (auto source : channel.second) {
      if (!uploadChartTiles(source.second.get(), boatId, settings, &inserter)) {
        return false;
      }
    }
  }
  return true;
}

std::vector<std::pair<std::string, GetMinMaxTime>>
  getMinMaxTimes(
      const std::map<std::string, std::shared_ptr<DispatchData>>& sources) {
  std::vector<std::pair<std::string, GetMinMaxTime>> dst;
  dst.reserve(sources.size());
  for (const auto& source: sources) {
    GetMinMaxTime minMaxTime;
    source.second->visit(&minMaxTime);
    if (minMaxTime.valid) {
      dst.push_back({source.first, minMaxTime});
    }
  }
  return dst;
}

bool uploadChartSourceIndex(const NavDataset& data,
                            const std::string& boatId,
                            const ChartTileSettings& settings,
                            const std::shared_ptr<mongoc_database_t>& db) {
  const map<DataCode, map<string, shared_ptr<DispatchData>>> &allSources =
    data.dispatcher()->allSources();

  auto oid = makeOid(boatId);
  auto index = SHARED_MONGO_PTR(bson, bson_new());
  BSON_APPEND_OID(index.get(), "_id", &oid);
  {
    BsonSubDocument channels(index.get(), "channels");
    for (auto channel : allSources) {
      auto minMaxTimes = getMinMaxTimes(channel.second);
      if (0 < minMaxTimes.size()) {
        BsonSubDocument chanObj(
            &channels,
            wordIdentifierForCode(channel.first));

        for (const auto& mmt : minMaxTimes) {
          BsonSubDocument sourceObj(&chanObj, mmt.first.c_str());
          bsonAppend(&sourceObj, "first", mmt.second.first);
          bsonAppend(&sourceObj, "last", mmt.second.last);
          bsonAppend(&sourceObj, "priority",
              data.dispatcher()->sourcePriority(mmt.first));
        }
      }
    }
  }

  auto collection = UNIQUE_MONGO_PTR(
        mongoc_collection,
        mongoc_database_get_collection(
            db.get(),
            settings.sourceTable()
            .localName().c_str()));

   bool success = false;
   {
     WrapBson selector;
     BSON_APPEND_OID(&selector, "_id", &oid);
     bson_error_t error;
     auto concern = mongoWriteConcernForLevel(
         MONGOC_WRITE_CONCERN_W_DEFAULT);
     success = mongoc_collection_update(
        collection.get(),
        MONGOC_UPDATE_UPSERT,
        &selector,
        index.get(),
        concern.get(),
        &error);
     if (!success) {
       LOG(ERROR) << bsonErrorToString(error);
     }
   }
   return success;
}

}  // namespace sail
