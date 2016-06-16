#include <device/anemobox/Sources.h>

#include <gtest/gtest.h>

using namespace sail;

TEST(SourcesTest, InternalTest) {
  EXPECT_TRUE(sourceIsInternal("IMU"));
  EXPECT_FALSE(sourceIsExternal("IMU"));
  EXPECT_TRUE(sourceIsInternal("Internal GPS"));
  EXPECT_FALSE(sourceIsExternal("Internal GPS"));
  EXPECT_TRUE(sourceIsInternal("NavDevice"));
  EXPECT_FALSE(sourceIsExternal("NavDevice"));
}

TEST(SourceTest, ExternalTest) {
  EXPECT_TRUE(sourceIsExternal("NMEA2000/0"));
  EXPECT_FALSE(sourceIsInternal("NMEA0183: /dev/ttyMFD1"));
  EXPECT_TRUE(sourceIsExternal("CUPS"));
  EXPECT_FALSE(sourceIsInternal("CUPS"));
}
