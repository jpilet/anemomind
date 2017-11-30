#include <server/nautical/logimport/SailmonDbLoader.h>
#include <server/common/Env.h>
#include <server/nautical/logimport/LogAccumulator.h>
#include <server/common/logging.h>
#include <gtest/gtest.h>

using namespace sail;

std::string path = std::string(Env::SOURCE_DIR) + "/datasets/sailmon/sample.db";

TEST(SailmonDbLoaderTest, TimeTable) {
  auto db = openSailmonDb(path);
  EXPECT_TRUE(bool(db));
  auto corr = getSailmonTimeCorrectionTable(db.get());
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

  }
}

TEST(SailmonDbLoaderTest, SmokeTest) {
  LogAccumulator accumulator;
  EXPECT_TRUE(sailmonDbLoad(path, &accumulator));
}


