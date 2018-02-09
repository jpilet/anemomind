#include <server/nautical/tiles/MongoUtils.h>
#include <server/nautical/tiles/ChartTiles.h>
#include <functional>
#include <device/anemobox/Dispatcher.h>
#include <server/nautical/NavDataset.h>
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
                    const TimedSampleCollection<T>& data,
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
    const std::deque<TimedValue<T>>& values = data.samples();
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

struct TileMetaData {
  std::string what;
  std::string source;

  TileMetaData() {}
  TileMetaData(const std::string& w, const std::string& src)
    : what(w), source(src) {}

  bool defined() const {
    return !what.empty();
  }

  TileMetaData(DispatchData* x)
    : what(x->wordIdentifier()),
      source(x->source()) {}
};

class ChartSourceIndexBuilder {
 public:
  ChartSourceIndexBuilder(const std::string& boatid,
                          std::shared_ptr<Dispatcher> dispatcher);

  void add(const TileMetaData& metadata, TimeStamp first, TimeStamp last,
           int64_t tilecount);

  bool upload(const std::shared_ptr<mongoc_database_t>& db,
              const ChartTileSettings& settings);

 private:
  std::shared_ptr<Dispatcher> _dispatcher;
  WrapBson _index;
  BsonSubDocument _channels;
  std::string _boatId;
};

template<class T>
std::shared_ptr<bson_t> chartTileToBson(const ChartTile<T> tile,
                     const std::string& boatId,
                     const TileMetaData& data) {
  CHECK(data.defined());

  if (tile.samples.size() == 0) {
    return std::shared_ptr<bson_t>();
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
    bsonAppend(&key, "what", data.what);
    bsonAppend(&key, "source", data.source);
  }

  StatArrays arrays;

  for (const TimedValue<Statistics<T>>& stats : tile.samples) {
    stats.value.appendToArrays(&arrays);
  }

  bsonAppendCollection(result.get(), "mean", arrays.mean);
  bsonAppendCollection(result.get(), "min", arrays.min);
  bsonAppendCollection(result.get(), "max", arrays.max);
  bsonAppendCollection(result.get(), "count", arrays.count);

  return result;
}

template<class T>
bool uploadChartTile(const ChartTile<T> tile,
                     const TileMetaData& data,
                     const std::string& boatId,
                     const ChartTileSettings& settings,
                     BulkInserter *inserter) {
  std::shared_ptr<bson_t> obj = chartTileToBson(
      tile, boatId, data);
  if (obj) {
    return inserter->insert(obj);
  } else {
    // Uploading an empty tile does not make sense.
    // But it is not an error.
    return true;
  }
}

template <typename T>
struct ValuesWithMetaData {
  TileMetaData metadata;
  TimedSampleCollection<T> values;
};

struct LonAndLatValues {
  ValuesWithMetaData<Angle<double>> lon, lat;
};

LonAndLatValues splitGeoPositions(
    const TypedDispatchData<GeographicPosition<double>>* src) {
  LonAndLatValues dst;
  dst.lat.metadata = TileMetaData("latitude", src->source());
  dst.lon.metadata = TileMetaData("longitude", src->source());
  const auto& values = src->dispatcher()->values();
  for (auto x: values) {
    dst.lat.values.append(x.time, x.value.lat());
    dst.lon.values.append(x.time, x.value.lon());
  }
  return dst;
}

class UploadChartTilesVisitor : public DispatchDataVisitor {
 public:
  UploadChartTilesVisitor(const std::string& boatId,
                          const ChartTileSettings& settings,
                          BulkInserter *inserter,
                          ChartSourceIndexBuilder* index)
    : _boatId(boatId), _settings(settings),
    _inserter(inserter), _result(true), _index(index) { }

  template<class T>
  void makeTiles(
      const TileMetaData& tileMetaData,
      const TimedSampleCollection<T>& values) {
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

    uint64_t tileCount = 0;

    for (int zoom = _settings.lowestZoomLevel;
         zoom <= _settings.highestZoomLevel; zoom++) {
      int64_t firstTile = tileAt(firstTime, zoom);
      int64_t lastTile = tileAt(lastTime, zoom);

      map<int64_t, ChartTile<T>> tiles;

      for (int64_t tileno = firstTile; tileno <= lastTile; ++tileno) {
        ChartTile<T> tile;
        downSampleData(tileno, zoom, values, _settings,
                       (zoom == _settings.lowestZoomLevel ?
                           nullptr : &prevZoomTiles),
                       &tile);
        if (!tile.empty()) {
          ++tileCount;
          tiles[tileno] = tile;
          if (!uploadChartTile(
              tiles[tileno], tileMetaData, _boatId,
              _settings, _inserter)) {
            _result = false;
            return;
          }
        }
      }

      // Keep previous zoom tiles so that downSampleData can use them
      // to produce the next zoom level.
      prevZoomTiles.swap(tiles);
    }

    _index->add(tileMetaData, firstTime, lastTime, tileCount);
  }

  template<class T>
  void makeTilesFromDispatcher(
      TypedDispatchData<T>* data) {
    makeTiles(
        TileMetaData(data),
        data->dispatcher()->values());
  }

  virtual void run(DispatchAngleData *angle) {
    makeTilesFromDispatcher(angle);
  }
  virtual void run(DispatchVelocityData *velocity) {
    makeTilesFromDispatcher(velocity);
  }
  virtual void run(DispatchLengthData *length) {
    makeTilesFromDispatcher(length);
  }
  virtual void run(DispatchGeoPosData *pos) {
    LonAndLatValues s = splitGeoPositions(pos);
    makeTiles<Angle<double>>(s.lon.metadata, s.lon.values);
    makeTiles<Angle<double>>(s.lat.metadata, s.lat.values);
  }
  virtual void run(DispatchTimeStampData *timestamp) { /* nothing */ }
  virtual void run(DispatchAbsoluteOrientationData *orient) {
    ValuesWithMetaData<Angle<double>> yaw, pitch, roll;
    for (auto x : orient->dispatcher()->values()) {
      yaw.values.append(x.time, x.value.heading);
      roll.values.append(x.time, x.value.roll);
      pitch.values.append(x.time, x.value.pitch);
    }

    makeTiles<Angle<double>>(TileMetaData("yaw", orient->source()), yaw.values);
    makeTiles<Angle<double>>(TileMetaData("pitch", orient->source()), pitch.values);
    makeTiles<Angle<double>>(TileMetaData("roll", orient->source()), roll.values);
  }

  virtual void run(DispatchBinaryEdge *binary) { /* nothing */ }
  virtual void run(DispatchAngularVelocityData *av) {
    makeTilesFromDispatcher(av);
  }

  bool result() const { return _result; }

 private:
  const std::string& _boatId;
  const ChartTileSettings& _settings;
  BulkInserter *_inserter;
  bool _result;
  ChartSourceIndexBuilder* _index;
};

bool uploadChartTiles(DispatchData* data,
                      const std::string& boatId,
                      const ChartTileSettings& settings,
                      BulkInserter *db,
                      ChartSourceIndexBuilder* index) {
  UploadChartTilesVisitor visitor(boatId, settings, db, index);
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

  auto oid = makeOid(boatId);
  {
    WrapBson selector;
    {
      BsonSubDocument id(&selector, "_id");
      BSON_APPEND_OID(&id, "boat", &oid);
    }
    auto concern = nullptr;
    bson_error_t error;
    if (!mongoc_collection_remove(
        collection.get(),
        MONGOC_REMOVE_NONE,
        &selector,
        concern,
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
  ChartSourceIndexBuilder index(boatId, data.dispatcher());

  for (auto channel : allSources) {
    for (auto source : channel.second) {
      if (!uploadChartTiles(source.second.get(), boatId,
                            settings, &inserter, &index)) {
        return false;
      }
    }
  }
  return index.upload(db, settings);
}

ChartSourceIndexBuilder::ChartSourceIndexBuilder(
    const std::string& boatId, std::shared_ptr<Dispatcher> dispatcher)
  : _channels(&_index, "channels"), _dispatcher(dispatcher),
    _boatId(boatId) { }

void ChartSourceIndexBuilder::add(const TileMetaData& metadata,
                                  TimeStamp first, TimeStamp last,
                                  int64_t tileCount) {
  BsonSubDocument chanObj(&_channels, metadata.what.c_str());
  BsonSubDocument sourceObj(&chanObj, metadata.source.c_str());

  bsonAppend(&sourceObj, "first", first);
  bsonAppend(&sourceObj, "last", last);
  bsonAppend(&sourceObj, "priority",
             _dispatcher->sourcePriority(metadata.source));
  bsonAppend(&sourceObj, "tileCount", tileCount);
}

bool ChartSourceIndexBuilder::upload(
    const std::shared_ptr<mongoc_database_t>& db,
    const ChartTileSettings& settings) {
  _channels.finalize();
  auto oid = makeOid(_boatId);
  BSON_APPEND_OID(&_index, "_id", &oid);

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
     auto concern = nullptr;
     success = mongoc_collection_update(
        collection.get(),
        MONGOC_UPDATE_UPSERT,
        &selector,
        &_index,
        concern,
        &error);
     if (!success) {
       char* json = bson_as_canonical_extended_json(&_index, NULL);
       LOG(ERROR) << "for boat ID " << _boatId << ": "
         << bsonErrorToString(error) << "\nReplacement:\n" << json;
       bson_free(json);
     }
   }
   return success;
}

}  // namespace sail
