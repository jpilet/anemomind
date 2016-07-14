#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>
#include <server/common/Env.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/NavCompatibility.h>

using namespace sail;

using namespace sail::NavCompat;

TEST(ScreenRecordingTest, screenAtTest) {
  ScreenRecordingSimulator simulator;

  std::string nmeaFile(std::string(Env::SOURCE_DIR)
                       + std::string("/datasets/tinylog.txt"));

  simulator.prepare("", "");
  simulator.simulate(nmeaFile);

  NavDataset navs = LogLoader::loadNavDataset(nmeaFile);

  int n = getNavSize(navs);
  EXPECT_LE(10, n);
  EXPECT_LE(n, 11);
  int failureCount = 0;
  for (int i = 0; i < n; ++i) {
    ScreenInfo info;
    auto nav = getNav(navs, i);
    failureCount += simulator.screenAt(nav.time(), &info)? 0 : 1;
    EXPECT_NEAR((info.time - getNav(navs, i).time()).seconds(), 0, 2);
  }
  EXPECT_LE(failureCount, 2);
}
