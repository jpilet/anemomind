/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/FractalFlow.h>

using namespace sail;

namespace {
  void windFlowDemo001() {
    Length<double> ul = Length<double>::nauticalMiles(1000);
    Duration<double> ut = Duration<double>::days(7);

    auto flow = makeWindFlow001();
    flow.plotForPosition(0,
        Flow::ProjectedPosition{0.5*ul, 0.5*ul},
        0.0*ut, 1.0*ut);
  }
}

int main(int argc, const char **argv) {
  windFlowDemo001();
  return 0;
}
