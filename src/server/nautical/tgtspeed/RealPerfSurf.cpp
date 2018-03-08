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

RealPerfSurfResults optimizeRealPerfSurf(
    const NavDataset& src,
    const RealPerfSurfSettings& settings) {
  auto tws = src.samples<TWS>();
  auto twa = src.samples<TWA>();
  auto boatSpeed = src.samples<GPS_SPEED>();

  auto twsWrap = TwsWrap();
  auto twaWrap = TwaWrap();
  auto bsWrap = BoatSpeedWrap();

  TimedTuples::Settings tupleSettings;

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
      trTimedTuples<BaseWrap::Variant, 3>(tupleSettings),
      IntoCount());
}


} /* namespace sail */
