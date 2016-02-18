#include <gtest/gtest.h>
#include <server/common/Duration.h>
#include "Nav.h"
#include "NavBBox.h"
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>


using namespace sail;

namespace {
  Poco::Path getAllNavsPath() {
    return PathBuilder::makeDirectory(Env::SOURCE_DIR).
        pushDirectory("datasets").makeFile("allnavs.txt").get();
  }
}

TEST(NavTest, MaxSpeed) {
  Nav a, b, c;
  TimeStamp offset = TimeStamp::UTC(2015, 10, 6, 12, 9, 0);
  a.setTime(offset);
  a.setGpsSpeed(Velocity<double>::knots(3.0));
  b.setTime(offset + Duration<double>::minutes(12));
  b.setGpsSpeed(Velocity<double>::knots(2.1));
  c.setTime(offset + Duration<double>::minutes(24));
  c.setGpsSpeed(Velocity<double>::knots(30.1));
  EXPECT_EQ(1, findMaxSpeedOverGround(NavCollection::fromNavs({a, b, c})));
}

TEST(NavTest, SortedTest) {
  NavCollection navs = loadNavsFromText(getAllNavsPath().toString());
  EXPECT_TRUE(navs.size() > 3);

  EXPECT_TRUE(areSortedNavs(navs));
}

TEST(NavTest, NavBBoxTest) {
  NavCollection navs = loadNavsFromText(getAllNavsPath().toString(), true);
  EXPECT_TRUE(navs.size() > 3);

  Array<NavCollection> splitNavs = splitNavsByDuration(navs,
                                 Duration<double>::minutes(10));
  Array<NavBBox> boxes = calcNavBBoxes(splitNavs);

  int count = boxes.size();
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < count; j++) {
      EXPECT_TRUE((i == j) == boxes[i].intersects(boxes[j]));
    }
  }
}

