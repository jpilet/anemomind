/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <gtest/gtest.h>
#include <server/nautical/synthtest/NavalSimulationPrecomp.h>

using namespace sail;

TEST(LinearFitTest, BasicTest) {
  NavalSimulation sim = getNavSimFractalWindOriented();
  auto boatData = sim.boatData(0);
}


