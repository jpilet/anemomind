#include <server/nautical/Nav.h>
#include "ExSwitch.h"
#include <server/common/Duration.h>
#include <server/nautical/LocalRace.h>

using namespace sail;


void example005() // Plot a single trajectory
{
	Array<Nav> navs = loadNavsFromText(ALLNAVSPATH, false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(navs, Duration::minutes(10).getDurationSeconds());

	plotNavsEcefTrajectory(splitNavs.last()); 
}

void example008()
{
	Array<Nav> allNavs = loadNavsFromText(ALLNAVSPATH, false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs,
           Duration::minutes(10).getDurationSeconds());
	Array<Nav> navs = splitNavs.first();
	double spaceStep = 500; // metres
	double timeStep = Duration::minutes(10).getDurationSeconds();
	LocalRace race(navs, spaceStep, timeStep);
	race.makeSpatioTemporalPlot(navs);
}


int main(int argc, char **argv) {
  ExSwitch ex;
  ADDEXAMPLE(ex, example005, "Plot a trajectory from allnavs");
  ADDEXAMPLE(ex, example008, "Plot a trajectory within the grid of a local race.");
  ex.run(argc, argv);
  return 0;
}
