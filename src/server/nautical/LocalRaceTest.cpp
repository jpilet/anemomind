#include "LocalRace.h"
#include <gtest/gtest.h>
#include <server/common/Duration.h>

using namespace sail;

#ifdef TIMECONSUMING_TESTS

TEST(LocalRaceTest, InstationAndRegTest) {
  Array<Nav> allNavs = loadNavsFromText(ALLNAVSPATH, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
                                 Duration::minutes(10).getDurationSeconds());
  Array<Nav> navs = splitNavs.first();

  double spaceStep = 500; // metres
  double timeStep = Duration::minutes(10).getDurationSeconds();
  LocalRace race(navs, spaceStep, timeStep);
  Grid3d wind = race.getWindGrid();
  for (int i = 0; i < 3; i++) {
    arma::sp_mat reg = wind.makeFirstOrderReg(i);
    EXPECT_EQ(reg.n_cols, wind.getVertexCount());
    arma::mat X = arma::ones(reg.n_cols, 1);
    double err = arma::norm(reg*X, 2);
    EXPECT_NEAR(err, 0, 1.0e-6);
  }
}

#endif
