/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/logs/LogLoader.h>
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
}
