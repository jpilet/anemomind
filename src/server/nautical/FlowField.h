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
 *
 * Main application: To generate synthetic wind/current fields
 *   for use in test cases.
 */
class FlowField {
 public:

  // PRIVATE: (only public because the Json interface needs them)
        // Use raw doubles for the internal representation to facilitate
        // the math ops.
        typedef Vectorize<double, 2> InternalFlowVector;
        FlowField(Grid3d grid_, Array<InternalFlowVector> flow_) : _grid(grid_), _flow(flow_) {}




  typedef Vectorize<Velocity<double>, 2> FlowVector;

  FlowField() {}

  /*
   * Generate a random vector field by first assigning
   * a random flow at every vertex and then repeatedly smoothing
   * in space in time with a box filter (converges to Gaussian smoothing)
   */
  static FlowField generate(Span<Length<double> > xSpan,
                            Span<Length<double> > ySpan,
                            Span<Duration<double> > timeSpan,
                            Length<double> spaceRes,
                            Duration<double> timeRes,
                            FlowVector meanFlow,
                            Velocity<double> maxDif,
                            int spaceSmoothinIters, int timeSmoothingIters);

  // Look up
  FlowVector map(Length<double> x, Length<double> y, Duration<double> time) const;

  void plotTimeSlice(Duration<double> time) const;

  // Accessors used by the Json interface
  const Grid3d &grid() const {return _grid;}
  const Array<InternalFlowVector> &flow() const {return _flow;}
 private:

  FlowVector makeFlowVector(const InternalFlowVector &x) const {
    return FlowVector{Velocity<double>::metersPerSecond(x[0]),
                      Velocity<double>::metersPerSecond(x[1])};
  }

  // The grid, defining the field
  Grid3d _grid;

  // Sampled flows at the grid vertices
  Array<InternalFlowVector> _flow;
};

} /* namespace sail */

#endif /* FLOWFIELD_H_ */
