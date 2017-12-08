#include <server/nautical/logimport/SailmonDbLoader.h>
#include <server/nautical/BoatSpecificHacks.h>
#include <server/common/Env.h>
#include <server/nautical/logimport/LogAccumulator.h>
#include <server/common/logging.h>
#include <gtest/gtest.h>
#include <server/nautical/filters/SmoothGpsFilter.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/DownsampleGps.h>
#include <server/nautical/tiles/TileUtils.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/nautical/filters/GpsUtils.h>

using namespace sail;

std::string path = std::string(Env::SOURCE_DIR) + "/datasets/sailmon/sample.db";

/*TEST(SailmonDbLoaderTest, TimeTable) {
  auto db = openSailmonDb(path);
  EXPECT_TRUE(bool(db));
  auto corr = getSailmonTimeCorrectionTable(db);
  EXPECT_LE(100, corr.size());
  EXPECT_TRUE(std::is_sorted(corr.begin(), corr.end()));


  auto testData = corr;
  testData.resize(40);

  for (auto x: testData) {
    std::cout << "logTime=" << x.logTime << " absTime=" << x.absoluteTime.toString() << std::endl;
  }

  {
    TimeStamp t0 = estimateTime(corr, 3600619);
    TimeStamp t1 = TimeStamp::UTC(2017, 9, 29, 12, 33, 57);

    auto diff = t0 - t1;
    EXPECT_NEAR(diff.seconds(), 0.0, 1.0);
  }{
    TimeStamp t0 = estimateTime(corr, 3600619 - 4000);
    TimeStamp t1 = TimeStamp::UTC(2017, 9, 29, 12, 33, 57) - 4.0_s;

    auto diff = t0 - t1;
    EXPECT_NEAR(diff.seconds(), 0.0, 1.0);
  }{
    auto p = *(corr.end()-1);
    {
      auto t = estimateTime(corr, p.logTime);
      auto diff = t - p.absoluteTime;
      EXPECT_NEAR(diff.seconds(), 0.0, 1.0);
    }
    auto q = p;
    q.logTime += 4000;
    q.absoluteTime += 4.0_s;
    {
      auto t = estimateTime(corr, q.logTime);
      auto diff = t - q.absoluteTime;
      EXPECT_NEAR(diff.seconds(), 0.0, 1.0);
    }
  }
}

TEST(SailmonDbLoaderTest, SmokeTest) {
  LogAccumulator accumulator;
  EXPECT_TRUE(sailmonDbLoad(path, &accumulator));
  const auto& gpsPos = accumulator._GPS_POSsources;
  EXPECT_LT(0, gpsPos.size());
  const auto& gpsPosFirstSource = accumulator._GPS_POSsources.begin()->second;
  EXPECT_LT(0, gpsPosFirstSource.size());
  const auto& firstGpsPosSample = gpsPosFirstSource[0];

  TimeStamp t1 = TimeStamp::UTC(2017, 9, 29, 12, 33, 57);
  EXPECT_NEAR((t1 - firstGpsPosSample.time).seconds(), 0.0, 2.0);
  LOG(INFO) << "Lon " << firstGpsPosSample.value.lon().degrees() << " deg";
  LOG(INFO) << "Lat " << firstGpsPosSample.value.lat().degrees() << " deg";
}*/


NavDataset slice1(const NavDataset& ds) {
  auto marg = 8.0_s;
  auto a = TimeStamp::UTC(2017, 9, 25, 13, 14, 19);
  auto b = TimeStamp::UTC(2017, 9, 25, 13, 27, 0);
  auto middle = a + 0.5*(b - a);
  return ds.slice(
       middle - marg,
       middle + marg);

}

NavDataset slice2(const NavDataset& ds) {
  auto marg = 1.0_h;
  return ds.slice(
      TimeStamp::UTC(2017, 9, 25, 11, 14, 19) - marg,
      TimeStamp::UTC(2017, 9, 25, 11, 27, 0) + marg);
}

NavDataset slice3(const NavDataset& ds) {
  auto marg = 1.0_minutes;
  auto a = TimeStamp::UTC(2017, 9, 25, 13, 14, 19);
  auto b = TimeStamp::UTC(2017, 9, 25, 13, 27, 0);
  auto middle = a + 0.5*(b - a);
  return ds.slice(
       middle - marg,
       middle + marg);

}

TEST(SailmonDbLoaderTest, GpsTest) {
  std::string path25 = "/Users/jonas/data/boatsailmon/raw_log-2017-09-25.db";
  NavDataset current =   GpsUtils::deduplicateGpsPositions(
      LogLoader::loadNavDataset(path25));

  current = slice3(current);
  std::cout << "Dataset " << current.boundsAsString() << std::endl;
  saveDispatcher("/tmp/sailmonslice.log", *(current.dispatcher()));

  hack::SelectSources(&current);
  current = removeStrangeGpsPositions(current);
  auto minGpsSamplingPeriod = 0.01_s; // Should be enough, right?
  current = current.createMergedChannels(
      std::set<DataCode>{GPS_POS, GPS_SPEED, GPS_BEARING},
      minGpsSamplingPeriod);
  auto report = DOM::makeBasicHtmlPage(
      "SailmonDbLoaderTest::GpsTest",
      "/Users/jonas/", "sailmon");
  GpsFilterSettings gpsFilterSettings;
  current = filterNavs(current, &report, gpsFilterSettings);

}
