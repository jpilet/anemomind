/*
 * compareFilters.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: jonas
 */


#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/WGS84.h>
#include <server/common/LineKM.h>
#include <server/nautical/InvWGS84.h>

using namespace sail;

namespace {

  std::default_random_engine rng;

  NavDataset getPsarosTestData() {
    auto p = PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets")
      .pushDirectory("psaros33_Banque_Sturdza")
      .pushDirectory("2014")
      .pushDirectory("20140821").get();
    LogLoader loader;
    loader.load(p.toString());
    auto ds = loader.makeNavDataset().fitBounds();
    auto startAt = TimeStamp::UTC(2014, 8, 21, 16, 46, 30);
    return ds.sliceFrom(startAt);
  }

  GeographicPosition<double> makeRandomGpsPosition() {
    std::normal_distribution<double> distrib(0.0, 1.0);
    auto m = Length<double>::meters(1.0);
    double x = distrib(rng);
    double y = distrib(rng);
    double z = distrib(rng);
    Length<double> pos[3] = {x*m, y*m, z*m};
    auto geoPos = computeGeographicPositionFromXYZ(pos);
    geoPos.setAlt(0.0*m);
    return geoPos;
  }

  TimedSampleCollection<GeographicPosition<double> >::TimedVector applyOutliers(
      double rate, const TimedSampleRange<GeographicPosition<double> > &src) {

    std::uniform_real_distribution<double> distrib(0.0, 1.0);
    TimedSampleCollection<GeographicPosition<double> >::TimedVector corruptPositions;

    int n = src.size();

    for (int i = 0; i < n; i++) {
      const auto &x = src[i];
      if (distrib(rng) < rate) {
        corruptPositions.push_back(TimedValue<GeographicPosition<double> >(
            x.time, makeRandomGpsPosition()));
      } else {
        corruptPositions.push_back(x);
      }
    }
    return corruptPositions;
  }

  struct Setup {
    Setup(NavDataset ds, double rate) {
      std::string srcName = "corrupted";
      originalDataset = ds;
      originalSamples = ds.samples<GPS_POS>();
      corruptedSamples = applyOutliers(rate, originalSamples);
      corruptedDataset = ds.overrideChannels(
          srcName,
          {{GPS_POS, makeDispatchDataFromSamples<GPS_POS>(
              srcName, corruptedSamples)}});

    }

    Length<double> evaluate(const TimedSampleCollection<GeographicPosition<double> >
      &filteredPositions) const {

      int n = originalSamples.size();

      auto totalError = Length<double>::meters(0.0);
      int counter = 0;
      for (int i = 0; i < n; i++) {
        auto x = originalSamples[i];
        auto y = filteredPositions.nearestTimedValue(x.time);
        if (y.defined()) {
          totalError = totalError + distance(x.value, y.get().value);
          counter++;
        }
      }
      assert(60 < counter);
      return (1.0/counter)*totalError;
    }

    NavDataset originalDataset, corruptedDataset;
    TimedSampleRange<GeographicPosition<double> > originalSamples;
    TimedSampleCollection<GeographicPosition<double> >::TimedVector corruptedSamples;
  };

}

int main() {
  auto data = getPsarosTestData();

  int rateCount = 12;

  std::ofstream file("/tmp/outlier_sensitivity.txt");
  LineKM rates(0, rateCount-1, 0.0, 1.0);
  for (int i = 0; i < rateCount; i++) {
    double rate = rates(i);
    LOG(INFO) << "Trying with an outlier rate of " << rate;
    Setup setup(data, rate);

    GpsFilterSettings settings;
    settings.backend = GpsFilterSettings::Ceres;

    auto ceresResults = filterGpsData(setup.corruptedDataset, settings)
        .getGlobalPositions();

    settings.backend = GpsFilterSettings::Irls;

    auto irlsResults = filterGpsData(setup.corruptedDataset, settings)
        .getGlobalPositions();

    file << rate << " " << setup.evaluate(ceresResults).meters()
        << " " << setup.evaluate(irlsResults).meters() << "\n";
  }

  return 0;
}


