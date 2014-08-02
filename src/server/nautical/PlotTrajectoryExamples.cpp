#include "PlotTrajectoryExamples.h"
#include <server/common/Duration.h>
#include <server/common/Env.h>
#include <server/nautical/LocalRace.h>
#include <server/nautical/Nav.h>

namespace sail {

void ptexLastRace() { // Plot a single trajectory
  Array<Nav> navs = loadNavsFromText(
      std::string(Env::SOURCE_DIR) + "/datasets/allnavs.txt", false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(
      navs, Duration<>::minutes(10).seconds());

  plotNavsEcefTrajectory(splitNavs.last());
}

void ptexLocalRace() {
  Array<Nav> allNavs = loadNavsFromText(
      std::string(Env::SOURCE_DIR) + "/datasets/allnavs.txt", false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(
      allNavs, Duration<>::minutes(10).seconds());
  Array<Nav> navs = splitNavs.first();
  double spaceStep = 500; // metres
  double timeStep = Duration<>::minutes(10).seconds();
  LocalRace race(navs, spaceStep, timeStep);
  race.makeSpatioTemporalPlot(navs);
}

}  // namespace sail
