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
  LOG(INFO) << "Loaded " << corr.size() << " time correction pairs";
}

TEST(SailmonDbLoaderTest, SmokeTest) {
  LogAccumulator accumulator;
  EXPECT_TRUE(sailmonDbLoad(path, &accumulator));
}


