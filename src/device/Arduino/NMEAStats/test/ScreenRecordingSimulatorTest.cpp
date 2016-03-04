#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>
#include <server/common/Env.h>
#include <server/nautical/logs/LogLoader.h>
#include <server/nautical/NavCompatibility.h>

using namespace sail;

using namespace sail::NavCompat;

TEST(ScreenRecordingTest, screenAtTest) {
  ScreenRecordingSimulator simulator;

  std::string nmeaFile(std::string(Env::SOURCE_DIR)
                       + std::string("/datasets/tinylog.txt"));

  simulator.prepare("", "");
  simulator.simulate(nmeaFile);

  NavDataset navs = flattenAndSort(
      Array<ParsedNavs>{loadNavsFromNmea(nmeaFile, "")},
      ParsedNavs::makeGpsWindMask());

  int n = getNavSize(navs);
  EXPECT_EQ(5, n);
  for (int i = 0; i < n; ++i) {
    ScreenInfo info;
    EXPECT_TRUE(simulator.screenAt(getNav(navs, i).time(), &info));
    EXPECT_NEAR((info.time - getNav(navs, i).time()).seconds(), 0, 2);
  }
}
