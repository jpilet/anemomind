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


// These functions are just temporarily visible, to
// facilitate debugging.
HorizontalMotion<double> apparentWind(const Nav &nav);
HorizontalMotion<double> estimateRawTrueWind(const Nav &nav);
Angle<double> estimateRawTwa(const Nav &n);
Velocity<double> estimateRawTws(const Nav &n);



class TargetSpeedData {
 public:
  static Arrayd makeDefaultQuantiles();
  TargetSpeedData() {}
  TargetSpeedData(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      int binCount, Velocity<double> minTws, Velocity<double> maxTws,
      Arrayd quantiles = makeDefaultQuantiles());
  void plot();
  const HistogramMap &hist() const {return _hist;}

  Arrayd targetVmgForWindSpeed(Velocity<double> windSpeed) const;
 private:
  void init(Array<Velocity<double> > windSpeeds,
      Array<Velocity<double> > vmg,
      HistogramMap map,
      Arrayd quantiles);

  Velocity<double> wrap(double x) {return Velocity<double>::knots(x);}
  double unwrap(Velocity<double> x) {return x.knots();}
  std::function<double(Velocity<double>)> makeUnwrapper() {return [=](Velocity<double> x) {return unwrap(x);};}

  // All velocities are internally stored as [knots]
  // Such a convention is reasonable, since HistogramMap only
  // works with doubles.
  Arrayd _quantiles;
  HistogramMap _hist;
  Array<Arrayd> _medianValues;
};

// Function used by getUpwindVmg and getDownwindVmg.
Array<Velocity<double> > calcVmg(Array<Nav> navs, bool isUpwind);

// All navs are collected from upwind legs
Array<Velocity<double> > calcUpwindVmg(Array<Nav> navs);

// All navs are collected from downwind legs
Array<Velocity<double> > calcDownwindVmg(Array<Nav> navs);

// Tws
Array<Velocity<double> > estimateTws(Array<Nav> navs);


// Pack "upwind" and "downwind" tables into a TargetSpeedTable and
// save it to the given ostream, in binary/chunk format.
void saveTargetSpeedTableChunk(
    ostream *stream,
    const TargetSpeedData& upwind,
    const TargetSpeedData& downwind);

} /* namespace sail */

#endif /* TARGETSPEED_H_ */
