#include <server/nautical/logimport/iwatch.h>

#include <server/common/Env.h>
#include <server/nautical/logimport/LogAccumulator.h>

#include <gtest/gtest.h>

using namespace sail;

TEST(IwatchParseTest, ParseWindFile) {
  LogAccumulator accumulator;

  EXPECT_TRUE(parseIwatch(
          std::string(Env::SOURCE_DIR) + "/datasets/iwatch_wind.json",
      &accumulator));

  EXPECT_EQ(1407, accumulator._AWAsources["iWatch"].size());
  EXPECT_EQ(1407, accumulator._AWSsources["iWatch"].size());
}

