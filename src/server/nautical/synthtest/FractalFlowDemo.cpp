/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/FractalFlow.h>

using namespace sail;

namespace {
  void windFlowDemo001(bool useWind) {
    Length<double> ul = Length<double>::nauticalMiles(1000);
    Duration<double> ut = Duration<double>::days(7);

    auto wcpair = makeWindCurrentPair001();
    auto flow = (useWind? wcpair.wind : wcpair.current);
    flow.plotForPosition(
    Flow::ProjectedPosition{0.5*ul, 0.5*ul},
        0.0*ut, 1.0*ut);


    Span<Length<double> > span(0.2*ul, 0.4*ul);
    Span<Length<double> > spans[2] = {span, span};
    BBox<Length<double>, 2> box(spans);
    Length<double> spacing = 0.01*ul;
    flow.plotVectorField(Duration<double>::days(5), box, spacing);
  }
}

int main(int argc, const char **argv) {
  if (argc <= 1) {
    std::cout << "Provide arguments\n wind: To display some wind\n current: To display some current\n";
  } else {
    for (int i = 1; i < argc; i++) {
      std::string s(argv[i]);
      if (s == "wind") {
        windFlowDemo001(true);
      } else if (s == "current") {
        windFlowDemo001(false);
      } else {
        std::cout << "Bad argument: " << s << std::endl;
        return -1;
      }
    }
  }
  return 0;
}
