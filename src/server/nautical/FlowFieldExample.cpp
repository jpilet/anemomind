/*
 *  Created on: 2014-05-23
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/FlowField.h>




using namespace sail;

namespace {
void makeAndDrawVectorField(FlowField::FlowVector meanFlow, Velocity<double> maxDif) {
    Span<Length<double> > spaceSpan(Length<double>::nauticalMiles(-2),
                                    Length<double>::nauticalMiles(2));
    Span<Length<double> > xSpan = spaceSpan;
    Span<Length<double> > ySpan = spaceSpan;
    Span<Duration<double> > timeSpan
      = Span<Duration<double> >(Duration<double>::hours(1.0), Duration<double>::hours(2.0));
    Length<double> spaceRes = Length<double>::meters(500);
    Duration<double> timeRes = Duration<double>::minutes(30);

    int spaceSmoothingIters = 3;
    int timeSmoothingIters = 3;

    FlowField ff = FlowField::generate(xSpan, ySpan, timeSpan,
        spaceRes, timeRes, meanFlow,
        maxDif,
        spaceSmoothingIters, timeSmoothingIters);
    Duration<double> time = Duration<double>::hours(1.2);
    ff.plotTimeSlice(time);
  }

}


int main() {
  makeAndDrawVectorField(
      FlowField::FlowVector{Velocity<double>::knots(0.5), Velocity<double>::knots(0.889)},
      Velocity<double>::knots(2.0));
  return 0;
}


