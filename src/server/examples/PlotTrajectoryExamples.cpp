#include <server/nautical/Nav.h>
#include <server/common/Duration.h>
#include <server/nautical/LocalRace.h>
#include "PlotTrajectoryExamples.h"

namespace sail {
  void ptex001() // Plot a single trajectory
  {
    Array<Nav> navs = loadNavsFromText(Nav::AllNavsPath, false);
    Array<Array<Nav> > splitNavs = splitNavsByDuration(navs, Duration::minutes(10).getDurationSeconds());

    plotNavsEcefTrajectory(splitNavs.last());
  }

  void ptex002()
  {
    Array<Nav> allNavs = loadNavsFromText(Nav::AllNavsPath, false);
    Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
             Duration::minutes(10).getDurationSeconds());
    Array<Nav> navs = splitNavs.first();
    double spaceStep = 500; // metres
    double timeStep = Duration::minutes(10).getDurationSeconds();
    LocalRace race(navs, spaceStep, timeStep);
    race.makeSpatioTemporalPlot(navs);
  }
}


