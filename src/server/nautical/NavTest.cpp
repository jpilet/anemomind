#include <gtest/gtest.h>
#include <server/common/Duration.h>
#include <server/nautical/NavCompatibility.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>


using namespace sail;
using namespace NavCompat;

namespace {
  Poco::Path getAllNavsPath() {
    return PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").makeFile("allnavs.txt").get();
  }
}

TEST(NavTest, MaxSpeed) {
  Nav a, b, c;
  auto pos = GeographicPosition<double>(Angle<double>::degrees(45), Angle<double>::degrees(45));
  TimeStamp offset = TimeStamp::UTC(2015, 10, 6, 12, 9, 0);
  a.setTime(offset);
  a.setGpsSpeed(Velocity<double>::knots(3.0));
  a.setGeographicPosition(pos);
  b.setTime(offset + Duration<double>::minutes(12));
  b.setGpsSpeed(Velocity<double>::knots(2.1));
  b.setGeographicPosition(pos);
  c.setTime(offset + Duration<double>::minutes(24));
  c.setGpsSpeed(Velocity<double>::knots(30.1));
  c.setGeographicPosition(pos);

  Array<Nav> navs = {a, b, c};
  EXPECT_EQ(1, findMaxSpeedOverGround(navs));
}
