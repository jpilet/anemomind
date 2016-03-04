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
#include <server/nautical/NavCompatibility.h>

namespace sail {




class TargetSpeed {
 public:
  static Arrayd makeDefaultQuantiles();
  TargetSpeed(bool isUpwind_, Array<Velocity<double> > tws, Array<Velocity<double> > vmg,
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
Array<Velocity<double> > calcVmg(NavDataset navs, bool isUpwind);

// Using the TWA from file
Array<Velocity<double> > calcExternalVmg(NavDataset navs, bool isUpwind);

// All navs are collected from upwind legs
Array<Velocity<double> > calcUpwindVmg(NavDataset navs);

// All navs are collected from downwind legs
Array<Velocity<double> > calcDownwindVmg(NavDataset navs);

// Tws
Array<Velocity<double> > estimateTws(NavDataset navs);

// Tws from file, reliable.
Array<Velocity<double> > estimateExternalTws(NavDataset navs);

void saveTargetSpeedTableChunk(
    ostream *stream,
    const TargetSpeed& upwind,
    const TargetSpeed& downwind);

} /* namespace sail */

#endif /* TARGETSPEED_H_ */
