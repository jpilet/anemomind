/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/LogUtils.h>
#include <device/anemobox/logger/LogLoader.h>
#include <server/nautical/TestDataFinder.h>

TEST(LogUtilsTest, TestLoadTiny) {
  sail::Dispatcher d;
  EXPECT_EQ(0, d.allSources().size());
  auto status = sail::LogUtils::loadAll(sail::findTestDataPath("tiny_unittest_log"), &d);
  EXPECT_EQ(status.successCount, 1);
  EXPECT_EQ(status.failureCount, 0);
  std::cout << status << std::endl;
  EXPECT_LT(0, d.allSources().size());
}
