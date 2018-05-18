#ifndef NAUTICAL_TILES_CHART_TILES_H
#define NAUTICAL_TILES_CHART_TILES_H

#include <map>
#include <string>
#include <set>

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

  MongoTableName table() const {
    return MongoTableName(dbName, chartTileTable);
  }

  MongoTableName sourceTable() const {
    return MongoTableName(dbName, chartTileSourceTable);
  }
private:
  std::string chartTileTable = "charttiles";
  std::string chartTileSourceTable = "chartsources";
};

bool uploadChartTiles(const NavDataset& data,
                      const std::string& boatId,
                      const ChartTileSettings& settings,
                      const std::shared_ptr<mongoc_database_t>& db);

struct StatArrays {
  std::vector<double> min, max, mean;
  std::vector<int64_t> count;
};

template <typename T> struct Statistics {
  MeanAndVar stats;

  void add(T x) { stats.add(unit(x)); }

  Statistics<T> operator +(const Statistics<T>& other) const {
    return Statistics<T>{ stats + other.stats };
  }

  static double unit(Velocity<> x) { return x.knots(); }
  static double unit(Length<> x) { return x.meters(); }
  static double unit(AngularVelocity<> x) { return x.degreesPerSecond(); }

  void appendToArrays(const string& /*what*/, StatArrays* arrays) const {
    if (stats.count() > 0) {
      arrays->count.push_back(1);
      arrays->mean.push_back(stats.mean());
      arrays->max.push_back(stats.max());
      arrays->min.push_back(stats.min());
    } else {
      arrays->count.push_back(0);
      arrays->mean.push_back(0);
      arrays->max.push_back(0);
      arrays->min.push_back(0);
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

  void appendToArrays(const string& what, StatArrays* arrays) const {
    static const std::set<std::string> negativeAngles {
      "awa", "twa", "rudderAngle", "pitch", "roll" };

    if (count > 0 && vectorSum.norm() > 0.01_kn) {
      arrays->count.push_back(static_cast<long long>(count));

      double value = 
        (negativeAngles.find(what) != negativeAngles.end() ?
         vectorSum.angle().normalizedAt0().degrees()
         : vectorSum.angle().positiveMinAngle().degrees());

      arrays->mean.push_back(value);
    } else {
      arrays->count.push_back(0);
      arrays->mean.push_back(0);
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
