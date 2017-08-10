#ifndef NAUTICAL_TILES_CHART_TILES_H
#define NAUTICAL_TILES_CHART_TILES_H

#include <map>
#include <string>

#include <device/anemobox/TimedSampleCollection.h>
#include <server/common/MeanAndVar.h>
#include <server/nautical/tiles/MongoUtils.h>

namespace sail {

class NavDataset;

struct ChartTileSettings {
  // These values should match those in dynloader.js
  int samplesPerTile = 512;
  int lowestZoomLevel = 9; // 2^9 = 512 seconds
  int highestZoomLevel = 28; // 2^28 seconds = about 10 years
  std::string dbName = "anemomind-dev";
  std::string chartTileTable = "charttiles";
  std::string chartTileSourceTable = "chartsources";

  std::string table() const { return dbName + "." + chartTileTable; }
  std::string sourceTable() const { return dbName + "." + chartTileSourceTable; }
};

bool uploadChartTiles(const NavDataset& data,
                      const std::string& boatId,
                      const ChartTileSettings& settings,
                      mongoc_database_t* db);

bool uploadChartSourceIndex(const NavDataset& data,
                            const std::string& boatId,
                            const ChartTileSettings& settings,
                            mongoc_database_t* db);


std::shared_ptr<mongo::BSONArrayBuilder> getBuilder(
    const std::string& key,
    std::map<std::string, std::shared_ptr<mongo::BSONArrayBuilder>>* arrays);

template <typename T> struct Statistics {
  MeanAndVar stats;

  void add(T x) { stats.add(unit(x)); }

  Statistics<T> operator +(const Statistics<T>& other) const {
    return Statistics<T>{ stats + other.stats };
  }

  static double unit(Velocity<> x) { return x.knots(); }
  static double unit(Length<> x) { return x.meters(); }

  void appendToArrays(std::map<std::string,
                     std::shared_ptr<mongo::BSONArrayBuilder>>* arrays) const {
    if (stats.count() > 0) {
      getBuilder("count", arrays)->append(1);
      getBuilder("mean", arrays)->append(stats.mean());
      getBuilder("max", arrays)->append(stats.max());
      getBuilder("min", arrays)->append(stats.min());
    } else {
      getBuilder("count", arrays)->append(0);
      getBuilder("mean", arrays)->append(0);
      getBuilder("max", arrays)->append(0);
      getBuilder("min", arrays)->append(0);
    }
  }
};

template <> struct Statistics<Angle<double>> {
  Statistics<Angle<double>>()
    : count(0), vectorSum(HorizontalMotion<double>::zero()) { }

  void add(Angle<> angle) {
    ++count;
    vectorSum += HorizontalMotion<double>::polar(Velocity<>::knots(1.0), angle);
  }

  Statistics<Angle<double>> operator + (
      const Statistics<Angle<double>>& other) const {
    Statistics<Angle<double>> result;
    result.count = other.count + count;
    result.vectorSum = other.vectorSum + vectorSum;
    return result;
  }

  void appendToArrays(std::map<std::string,
                     std::shared_ptr<mongo::BSONArrayBuilder>>* arrays) const {
    for (std::string key : {"count", "mean" }) {
      if (arrays->find(key) == arrays->end()) {
        (*arrays)[key] = std::make_shared<mongo::BSONArrayBuilder>();
      }
    }
    if (count > 0 && vectorSum.norm() > 0.01_kn) {
      (*arrays)["count"]->append(static_cast<long long>(count));
      (*arrays)["mean"]->append( vectorSum.angle().degrees());
    } else {
      (*arrays)["count"]->append(0);
      (*arrays)["mean"]->append(0);
    }
  }

  int64_t count;
  HorizontalMotion<double> vectorSum;
};

template <typename T>
struct ChartTile {
  int zoom;
  int64_t tileno;
  TimedSampleCollection<Statistics<T>> samples;

  bool empty() const { return samples.size() == 0; }
};

}  // namespace sail

#endif
