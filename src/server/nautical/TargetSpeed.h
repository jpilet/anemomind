/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Port of Matlab routines for target speed computation,
 *  see e-mail on 2014-05-08 with subject "What I used to produce Irene perts last year".
 */

#ifndef TARGETSPEED_H_
#define TARGETSPEED_H_

#include <server/nautical/Nav.h>
#include <server/common/Histogram.h>

namespace sail {

class TargetSpeedData {
 public:
  static Arrayd makeDefaultQuantiles();
  TargetSpeedData() {}
  TargetSpeedData(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      HistogramMap map = HistogramMap(),
      Arrayd quantiles = makeDefaultQuantiles());
  TargetSpeedData(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      int binCount,
      Arrayd quantiles = makeDefaultQuantiles());

 private:
  void init(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      HistogramMap map,
      Arrayd quantiles);

  HistogramMap _hist;
  Array<Array<Velocity<double> > > _medianValues;
};

// Function used by getUpwindVmg and getDownwindVmg.
Array<Velocity<double> > getVmg(Array<Nav> navs, bool isUpwind);

// All navs are collected from upwind legs
Array<Velocity<double> > getUpwindVmg(Array<Nav> navs);

// All navs are collected from downwind legs
Array<Velocity<double> > getDownwindVmg(Array<Nav> navs);


} /* namespace sail */

#endif /* TARGETSPEED_H_ */
