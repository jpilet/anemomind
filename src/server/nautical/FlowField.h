/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *
 *
 */

#include <server/math/Grid.h>
#include <server/common/PhysicalQuantity.h>

#ifndef FLOWFIELD_H_
#define FLOWFIELD_H_

namespace sail {

/*
 * Models a function: f : R³ -> R²
 *
 * that maps {x-pos, y-pos, time} -> {x-vel, y-vel},
 * e.g. to model the the wind or current at different locations
 * of the race area. This class is convenient to use together with
 * the GeographicReference class that maps between GeographicPosition:s and
 * local x/y-positions in a race area.
 */
class FlowField {
 public:
  typedef Vectorize<Velocity<double>, 2> FlowVector;

  FlowField(Span<Length<double> > xSpan,
            Span<Length<double> > ySpan,
            Span<Duration<double> > timeSpan);

  // Look up
  FlowVector map(Length<double> x, Length<double> y, Duration<double> time);
 private:
  void init(Grid3d grid, Array<FlowVector> flowVecs);

  // The grid, defining the field
  Grid3d _grid;

  // Sampled flows at the grid vertices
  Array<FlowVector> _flow;
};

} /* namespace sail */

#endif /* FLOWFIELD_H_ */
