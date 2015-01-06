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
    flow.plotForPosition(
    Flow::ProjectedPosition{0.5*ul, 0.5*ul},
        0.0*ut, 1.0*ut);


    Span<Length<double> > span(0.2*ul, 0.4*ul);
    Span<Length<double> > spans[2] = {span, span};
    BBox<Length<double>, 2> box(spans);
    Length<double> spacing = 0.01*ul;
    flow.plotVectorField(Duration<double>::days(3), box, spacing);
  }
}

int main(int argc, const char **argv) {
  windFlowDemo001();
  return 0;
}
