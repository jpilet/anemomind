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


TEST(NavTest, SortedTest) {
  Array<Nav> navs = loadNavsFromText(getAllNavsPath().toString());
  EXPECT_TRUE(navs.size() > 3);

  EXPECT_TRUE(areSortedNavs(navs));
}

TEST(NavTest, NavBBoxTest) {
  Array<Nav> navs = loadNavsFromText(getAllNavsPath().toString(), true);
  EXPECT_TRUE(navs.size() > 3);

  Array<Array<Nav> > splitNavs = splitNavsByDuration(navs,
                                 Duration<double>::minutes(10));
  Array<NavBBox> boxes = calcNavBBoxes(splitNavs);

  int count = boxes.size();
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < count; j++) {
      EXPECT_TRUE((i == j) == boxes[i].intersects(boxes[j]));
    }
  }
}

