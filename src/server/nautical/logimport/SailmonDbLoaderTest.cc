#include <server/nautical/logimport/SailmonDbLoader.h>

#include <server/common/env.h>
#include <server/nautical/logimport/LogAccumulator.h>

#include <gtest/gtest.h>

using namespace sail;

TEST(SailmonDbLoaderTest, SmokeTest) {
  std::string path = std::string(Env::SOURCE_DIR) + "/datasets/sailmon/sample.db";
  LogAccumulator accumulator;
  EXPECT_TRUE(sailmonDbLoad(path, &accumulator));
}


