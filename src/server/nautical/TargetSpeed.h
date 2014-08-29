/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Port of Matlab routines for target speed computation,
 *  see e-mail on 2014-05-08 with subject "What I used to produce Irene perts last year".
 */

#ifndef TARGETSPEED_H_
#define TARGETSPEED_H_

#include <ostream>
#include <server/nautical/Nav.h>
#include <server/common/Histogram.h>

namespace sail {




class RefImplTgtSpeed {
 public:
  static Arrayd makeDefaultQuantiles();
  RefImplTgtSpeed(bool isUpwind_, Array<Velocity<double> > tws, Array<Velocity<double> > vmg,
          Array<Velocity<double> > bounds, Arrayd quantiles_ = makeDefaultQuantiles());

  Array<Velocity<double> > binCenters;
  Array<Array<Velocity<double> > > medianValues;
  bool isUpwind;
  Arrayd quantiles;

  int quantileCount() const {
    return quantiles.size();
  }

  void plot();
};

Array<Velocity<double> > makeBoundsFromBinCenters(int binCount,
    Velocity<double> minBinCenter,
    Velocity<double> maxBinCenter);



// Function used by getUpwindVmg and getDownwindVmg.
Array<Velocity<double> > calcVmg(Array<Nav> navs, bool isUpwind);

// Using the TWA from file
Array<Velocity<double> > calcExternalVmg(Array<Nav> navs, bool isUpwind);

Array<Velocity<double> > calcExternalVmgUnsigned(Array<Nav> navs);

// All navs are collected from upwind legs
Array<Velocity<double> > calcUpwindVmg(Array<Nav> navs);

// All navs are collected from downwind legs
Array<Velocity<double> > calcDownwindVmg(Array<Nav> navs);

// Tws
Array<Velocity<double> > estimateTws(Array<Nav> navs);

// Tws from file, reliable.
Array<Velocity<double> > estimateExternalTws(Array<Nav> navs);


// Pack "upwind" and "downwind" tables into a TargetSpeedTable and
// save it to the given ostream, in binary/chunk format.
class TargetSpeedData;
void saveTargetSpeedTableChunk(
    ostream *stream,
    const TargetSpeedData& upwind,
    const TargetSpeedData& downwind);


void saveTargetSpeedTableChunk(
    ostream *stream,
    const RefImplTgtSpeed& upwind,
    const RefImplTgtSpeed& downwind);

} /* namespace sail */

#endif /* TARGETSPEED_H_ */
