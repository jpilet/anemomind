/*
 * calibexamples.cpp
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#include "calibexamples.h"
#include "Env.h"
#include "../common/Nav.h"
#include "../common/Duration.h"
#include "../math/LocalRace.h"

namespace sail
{

int main()
{
	calibexample001();
	return 0;
}

void calibexample001()
{
	SysEnv env;
	Array<Nav> allNavs = loadNavsFromText(env.dataset.cat("allnavs.txt").str(), false);
	Array<Array<Nav> > splitNavs = splitNavsByDuration(allNavs, Duration::minutes(10).getDurationSeconds());
	Array<Nav> navs = splitNavs.first();

	double spaceStep = 500; // metres
	double timeStep = Duration::minutes(10).getDurationSeconds();
	LocalRace race(navs, spaceStep, timeStep);

	// More to be added here but not yet...
}

} /* namespace sail */
