#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <device/Arduino/NMEAStats/test/ScreenRecordingSimulator.h>
#include <server/common/Env.h>
#include <server/nautical/NavNmea.h>

using namespace sail;

TEST(ScreenRecordingTest, screenAtTest) {
  ScreenRecordingSimulator simulator;

  std::string nmeaFile(std::string(Env::SOURCE_DIR)
                       + std::string("/datasets/tinylog.txt"));

  simulator.prepare("", "");
  simulator.simulate(nmeaFile);

  Array<Nav> navs = flattenAndSort(
      Array<ParsedNavs>{loadNavsFromNmea(nmeaFile, "")},
      ParsedNavs::makeGpsWindMask());

  EXPECT_EQ(5, navs.size());
  for (int i = 0; i < navs.size(); ++i) {
    ScreenInfo info;
    EXPECT_TRUE(simulator.screenAt(navs[i].time(), &info));
    EXPECT_NEAR((info.time - navs[i].time()).seconds(), 0, 2);
  }
}
