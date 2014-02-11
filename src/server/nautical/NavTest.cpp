#include <gtest/gtest.h>
#include <server/common/Duration.h>
#include "Nav.h"
#include "NavBBox.h"

using namespace sail;

// These tests read lots data from disk, which
// is a bit slow, even with SSD disk ;-)
// We may not want to run them every time.
#ifdef TIMECONSUMING_TESTS

TEST(NavTest, SortedTest) {
  Array<Nav> navs = loadNavsFromText(ALLNAVSPATH);
  EXPECT_TRUE(navs.size() > 3);

  EXPECT_TRUE(areSortedNavs(navs));
}

TEST(NavTest, NavBBoxTest) {
  Array<Nav> navs = loadNavsFromText(ALLNAVSPATH, true);
  EXPECT_TRUE(navs.size() > 3);

  Array<Array<Nav> > splitNavs = splitNavsByDuration(navs,
                                 Duration::minutes(10).getDurationSeconds());
  Array<NavBBox> boxes = calcNavBBoxes(splitNavs);

  int count = boxes.size();
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < count; j++) {
      EXPECT_TRUE((i == j) == boxes[i].intersects(boxes[j]));
    }
  }
}

#endif
