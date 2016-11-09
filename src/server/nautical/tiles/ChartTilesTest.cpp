#include <server/nautical/tiles/ChartTiles.h>

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <device/anemobox/FakeClockDispatcher.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <server/nautical/NavDataset.h>

using testing::Return;
using testing::_;

inline bool operator == (const mongo::Query& a, const mongo::Query& b) {
  return true;
}

namespace sail {

TEST(ChartTiles, VelocityStatTest) {
  Statistics<Velocity<>> vstatsA, vstatsB;

  vstatsA.add(Velocity<>::knots(3));
  vstatsA.add(Velocity<>::knots(4));
  EXPECT_EQ(2, vstatsA.stats.count());
  EXPECT_NEAR(3.5, vstatsA.stats.mean(), 1e-5);

  vstatsB.add(Velocity<>::knots(4));
  vstatsB.add(Velocity<>::knots(5));
  EXPECT_EQ(2, vstatsB.stats.count());
  EXPECT_NEAR(4.5, vstatsB.stats.mean(), 1e-5);

  Statistics<Velocity<>> sum = vstatsA + vstatsB;
  EXPECT_EQ(4, sum.stats.count());
  EXPECT_NEAR((3 + 4 + 4 + 5) / 4.0, sum.stats.mean(), 1e-5);
}

TEST(ChartTiles, AngleStatTest) {
  Statistics<Angle<>> astatsA, astatsB;

  astatsA.add(Angle<>::degrees(30));
  astatsA.add(Angle<>::degrees(40));
  astatsB.add(Angle<>::degrees(40));
  astatsB.add(Angle<>::degrees(50));

  Statistics<Angle<>> sum = astatsA + astatsB;
  EXPECT_EQ(4, sum.count);
  EXPECT_NEAR(40, sum.vectorSum.angle().degrees(), 1e-4);
}

class MockDBClientConnection : public mongo::DBClientConnection {
 public:
  MOCK_METHOD6(update,
               void(const std::string& ns,
                    mongo::Query query,
                    mongo::BSONObj obj,
                    bool upsert,
                    bool multi,
                    const mongo::WriteConcern* wc));

  // Mocking runCommand is necessary to mock getLastError().
  MOCK_METHOD4(runCommand, bool(const std::string& dbname,
                                const mongo::BSONObj& cmd,
                                mongo::BSONObj& info,
                                int options));
};

MATCHER_P3(tileQuery, boat, zoom, tileno, "") {
  // arg is a mongo::Query
  return 
    boat == arg.obj["_id"]["boat"].String()
    && zoom == arg.obj["_id"]["zoom"].Number()
    && tileno == arg.obj["_id"]["tileno"].Number();
}

MATCHER_P2(numSamplesWithCount, numSamples, count, "") {
  // arg is a mongo::BSONObj
  std::vector<mongo::BSONElement> samples = arg["samples"].Array();
  if (samples.size() != numSamples) {
    return false;
  }

  for (int i = 0; i < numSamples; ++i) {
    if (samples[i]["count"].Int() != count) {
      return false;
    }
  }
  return true;
};

TEST(ChartTiles, UploadOneTile) {
  std::shared_ptr<FakeClockDispatcher> disp =
    std::make_shared<FakeClockDispatcher>();

  ChartTileSettings settings;
  const int zoom = 7;
  settings.lowestZoomLevel = zoom;
  settings.highestZoomLevel = zoom;
  settings.samplesPerTile = 8;

  // Align with tile start
  TimeStamp base = TimeStamp::fromMilliSecondsSince1970((34 << zoom) * 1000);
  disp->setTime(base);

  // These measurements should fill exactly 1 tile
  for (int i = 0; i < settings.samplesPerTile; ++i) {
    disp->publishValue(GPS_SPEED, "testSource", Velocity<>::knots(5 + (i % 16)));
    disp->advance(Duration<>::seconds(double(1 << zoom) / settings.samplesPerTile));
  }

  NavDataset ds(disp);

  MockDBClientConnection db;
  EXPECT_CALL(db, runCommand(_, _, _, _)).WillRepeatedly(Return(true));

  EXPECT_CALL(db,
              update("anemomind-dev.charttiles",
                     tileQuery("fakeboatid", zoom, 34),
                     numSamplesWithCount(settings.samplesPerTile, 1),
                     true, false, nullptr));

  uploadChartTiles(ds, "fakeboatid", settings, &db);
}

TEST(ChartTiles, UploadTwoTilesPlusOneCombined) {
  std::shared_ptr<FakeClockDispatcher> disp =
    std::make_shared<FakeClockDispatcher>();

  ChartTileSettings settings;
  const int zoom = 7;
  settings.lowestZoomLevel = zoom;
  settings.highestZoomLevel = zoom + 1;
  settings.samplesPerTile = 8;

  // Align with tile start
  TimeStamp base = TimeStamp::fromMilliSecondsSince1970((34 << zoom) * 1000);
  disp->setTime(base);

  // These measurements should fill exactly 2 tiles
  for (int i = 0; i < 2 * settings.samplesPerTile; ++i) {
    disp->publishValue(GPS_SPEED, "testSource", Velocity<>::knots(5 + (i % 16)));
    disp->advance(Duration<>::seconds(double(1 << zoom) / settings.samplesPerTile));
  }

  NavDataset ds(disp);

  MockDBClientConnection db;
  EXPECT_CALL(db, runCommand(_, _, _, _)).WillRepeatedly(Return(true));

  EXPECT_CALL(db, update("anemomind-dev.charttiles",
                         tileQuery("fakeboatid", zoom, 34),
                         numSamplesWithCount(settings.samplesPerTile, 1),
                         true, false, nullptr));

  EXPECT_CALL(db, update("anemomind-dev.charttiles",
                         tileQuery("fakeboatid", zoom, 35),
                         numSamplesWithCount(settings.samplesPerTile, 1),
                         true, false, nullptr));

  // on next zoom level, there is only 1 tile, but for each tile bin
  // it combines 2 samples from preview tiles
  EXPECT_CALL(db, update("anemomind-dev.charttiles",
                         tileQuery("fakeboatid", zoom + 1, 34 / 2),
                         numSamplesWithCount(settings.samplesPerTile, 2),
                         true, false, nullptr));
  uploadChartTiles(ds, "fakeboatid", settings, &db);
}

}  // namespace sail
