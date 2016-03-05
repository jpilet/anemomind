/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

using namespace sail;

TEST(CsvTest, Rowdy) {
  auto data = LogLoader::loadNavDataset(
      PathBuilder::makeDirectory(Env::SOURCE_DIR)
      .pushDirectory("datasets")
      .pushDirectory("csvlog").get());
  EXPECT_EQ(data.samples<AWA>().size(), 6);
  EXPECT_EQ(data.samples<AWS>().size(), 6);
  EXPECT_EQ(data.samples<MAG_HEADING>().size(), 6);
  EXPECT_EQ(data.samples<WAT_SPEED>().size(), 6);
  EXPECT_EQ(data.samples<GPS_SPEED>().size(), 6);
  EXPECT_EQ(data.samples<GPS_BEARING>().size(), 6);
  EXPECT_EQ(data.samples<GPS_POS>().size(), 6);
  EXPECT_NEAR(data.samples<AWA>().last().value.degrees(), 20.380108772803382, 1.0e-5);
  EXPECT_NEAR(data.samples<GPS_POS>()[2].value.lon().degrees(), 6.999021099999999, 1.0e-5);

  TimeStamp time = data.samples<WAT_SPEED>()[2].time;
  auto expected = TimeStamp::UTC(2015, 9, 21, 13, 13, 23.80);

  EXPECT_NEAR(time.toMilliSecondsSince1970(), expected.toMilliSecondsSince1970(), 1001);
}
