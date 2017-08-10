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

bson_oid_t makeOid(const std::string& s) {
  bson_oid_t oid;
  bson_oid_init_from_string(&oid, s.c_str());
  return oid;
}


template<class T>
bool chartTileToBson(const ChartTile<T> tile,
                     const std::string& boatId,
                     const DispatchData* data) {
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
  withBsonSubDocument(result.get(), "_id", [&](bson_t* key) {
    auto oid = makeOid(boatId);
    BSON_APPEND_OID(key, "boat", &oid);
    BSON_APPEND_INT32(key, "zoom", tile.zoom);
    BSON_APPEND_INT64(key, "tileno", (long long) tile.tileno);
    BSON_APPEND_UTF8(key, "what", data->wordIdentifier().c_str());
    BSON_APPEND_UTF8(key, "source", data->source().c_str());
  });

  BSONArrayBuilder samples;
  StatArrays arrays;

  for (const TimedValue<Statistics<T>>& stats : tile.samples) {
    stats.value.appendToArrays(&arrays);
  }

  for (const auto& arr : arrays) {
    appendBsonPrimitiveArray();
    result.append(arr.first, arr.second->arr());
  }

  //result.append("samples", samples.arr());

  *obj = result.obj();
  return true;
}

template<class T>
bool uploadChartTile(const ChartTile<T> tile,
                     const DispatchData* data,
                     const std::string& boatId,
                     const ChartTileSettings& settings,
                     BulkInserter *inserter) {
  BSONObj obj;
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
                       (zoom == _settings.lowestZoomLevel ? nullptr : &prevZoomTiles),
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
          settings.table().c_str()));

  //MONGO_QUERY("_id.boat" << OID(boatId)),

  auto oid = makeOid(boatId);

  bson_t selector, id;
  bson_init(&selector);
  BSON_APPEND_DOCUMENT_BEGIN(&selector, "_id", &id);
  BSON_APPEND_OID(&id, "boat", oid);
  bson_append_document_end(&selector, &id);

  if (!mongoc_collection_remove(
      collection.get(),
      MONGOC_REMOVE_NONE,
      selector.get(),
      MONGOC_WRITE_CONCERN_W_UNACKNOWLEDGED(0),
      nullptr)) {
    LOG(ERROR) << "Failed to execute remove old chart tiles for boat, but we will continue.";
  }
  bson_destroy (&selector);

  BulkInserter inserter(settings.table(), 1000, db);

  for (auto channel : allSources) {
    for (auto source : channel.second) {
      if (!uploadChartTiles(source.second.get(), boatId, settings, &inserter)) {
        return false;
      }
    }
  }
  return true;
}

bool uploadChartSourceIndex(const NavDataset& data,
                            const std::string& boatId,
                            const ChartTileSettings& settings,
                            const std::shared_ptr<mongoc_database_t>& db) {
  BSONObjBuilder channels;
  const map<DataCode, map<string, shared_ptr<DispatchData>>> &allSources =
    data.dispatcher()->allSources();

  for (auto channel : allSources) {
    BSONObjBuilder chanObj;
    int numSources = 0;
    for (auto source : channel.second) {

      GetMinMaxTime minMaxTime;
      source.second->visit(&minMaxTime);
      if (!minMaxTime.valid) {
        continue;
      }
      BSONObjBuilder sourceObj;
      append(sourceObj, "first", minMaxTime.first);
      append(sourceObj, "last", minMaxTime.last);
      sourceObj.append("priority", data.dispatcher()->sourcePriority(source.first));

      ++numSources;
      chanObj.append(source.first, sourceObj.obj());
    }
    if (numSources > 0) {
      channels.append(wordIdentifierForCode(channel.first),
                      chanObj.obj());
    }
  }

  BSONObjBuilder index;
  index.append("_id", OID(boatId));
  index.append("channels", channels.obj());
  auto obj = index.obj();

  return safeMongoOps(
      "Inserting dispatcher index entry", db,
      [=](DBClientConnection *db) {
        db->update(settings.sourceTable(),
                   MONGO_QUERY("_id" << obj["_id"]),  // <-- what to update
                   obj,                         // <-- the new data
                   true,                        // <-- upsert
                   false);                      // <-- multi
      }
    );
}

}  // namespace sail
