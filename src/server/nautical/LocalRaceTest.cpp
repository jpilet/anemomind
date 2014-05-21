#include "LocalRace.h"
#include <gtest/gtest.h>
#include <server/common/Duration.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/NavNmea.h>

using namespace sail;

TEST(LocalRaceTest, InstationAndRegTest) {
  Poco::Path path = PathBuilder::makeDirectory(Env::SOURCE_DIR).
      pushDirectory("datasets").
      pushDirectory("regates").
      pushDirectory("champ_suisse_10_juillet_08").makeFile("IreneLog.txt").get();
  Array<Nav> allNavs = loadNavsFromNmea(path.toString(), Nav::debuggingBoatId()).navs();
  Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
                                 Duration<double>::minutes(10).seconds());
  Array<Nav> navs = splitNavs.first();

  double spaceStep = 500; // metres
  double timeStep = Duration<double>::minutes(10).seconds();
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

