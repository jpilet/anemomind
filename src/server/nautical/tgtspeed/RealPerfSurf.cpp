/*
 * RealPerfSurf.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 */

#include "RealPerfSurf.h"
#include <server/common/VariantIterator.h>
#include <server/common/TimedTuples.h>

namespace sail {

namespace {
  typedef VariantIteratorWrapper<
      0, Velocity<double>, Angle<double>, Velocity<double>> BaseWrap;

  typedef BaseWrap TwsWrap;
  typedef TwsWrap::NextType TwaWrap;
  typedef TwaWrap::NextType BoatSpeedWrap;

}

struct WindAndBoatSpeedSample {
  Velocity<double> tws;
  Angle<double> twa;
  Velocity<double> boatSpeed;
};

RealPerfSurfResults optimizeRealPerfSurf(
    const NavDataset& src,
    const RealPerfSurfSettings& settings) {
  auto tws = src.samples<TWS>();
  auto twa = src.samples<TWA>();
  auto boatSpeed = src.samples<GPS_SPEED>();

  auto twsWrap = TwsWrap();
  auto twaWrap = TwaWrap();
  auto bsWrap = BoatSpeedWrap();


  transduce(
      tws,
      trMap(twsWrap)
      |
      trMerge(
          twaWrap.wrapIterator(twa.begin()),
          twaWrap.wrapIterator(twa.end()))
      |
      trMerge(
          bsWrap.wrapIterator(boatSpeed.begin()),
          bsWrap.wrapIterator(boatSpeed.end()))
      |
      trFilter([settings](
          const TimedValue<IndexedValue<BaseWrap::Variant>>& x) {
        return settings.timeFilter(x.time);
      })
      |
      trTimedTuples<BaseWrap::Variant, 3>()
      |
      trFilter(IsShortTimedTuple(settings.timedTupleThreshold))
      |
      trMap([&](const BaseWrap::VariantTuple& vt) {
        WindAndBoatSpeedSample dst;
        auto twa = twaWrap.get(vt);
        auto tws = twsWrap.get(vt);
        auto boatSpeed = bsWrap.get(vt);
        dst.twa = twa.value;
        dst.tws = tws.value;
        dst.boatSpeed = boatSpeed.value;
        return TimedValue<WindAndBoatSpeedSample>(
            average({twa.time, tws.time, boatSpeed.time}),
            dst);
      }),
      IntoCount());

  return RealPerfSurfResults();
}


} /* namespace sail */
