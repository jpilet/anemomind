#ifndef NAUTICAL_TILES_CHART_TILES_H
#define NAUTICAL_TILES_CHART_TILES_H

#include <string>

#include <device/anemobox/TimedSampleCollection.h>
#include <server/common/MeanAndVar.h>
#include <server/nautical/tiles/MongoUtils.h>

namespace sail {

class NavDataset;

struct ChartTileSettings {
  int samplesPerTile = 256;
  int lowestZoomLevel = 7; // 2^7 = 128 seconds
  int highestZoomLevel = 28; // 2^28 seconds = about 10 years
  std::string dbName = "anemomind-dev";
  std::string chartTileTable = "charttiles";

  std::string table() const { return dbName + "." + chartTileTable; }
};

bool uploadChartTiles(const NavDataset& data,
                      const std::string& boatId,
                      const ChartTileSettings& settings,
                      mongo::DBClientConnection *db);


void appendStats(const MeanAndVar& stats, mongo::BSONObjBuilder* builder);

template <typename T> struct Statistics {
  MeanAndVar stats;

  void add(T x) { stats.add(unit(x)); }

  Statistics<T> operator +(const Statistics<T>& other) const {
    return Statistics<T>{ stats + other.stats };
  }

  static double unit(Velocity<> x) { return x.knots(); }
  static double unit(Length<> x) { return x.meters(); }

  void appendBson(mongo::BSONObjBuilder* builder) const {
    appendStats(stats, builder);
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

  void appendBson(mongo::BSONObjBuilder* builder) const {
    builder->append("count", (long long) count);
    if (count > 0 && vectorSum.norm() > 0.01_kn) {
      builder->append("mean", vectorSum.angle().degrees());
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
