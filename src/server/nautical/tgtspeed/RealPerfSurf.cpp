/*
 * RealPerfSurf.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: jonas
 */

#include "RealPerfSurf.h"
#include <server/common/VariantIterator.h>
#include <server/common/TimedTuples.h>
#include <server/nautical/tgtspeed/HexMesh.h>
#include <fstream>

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

template <typename T>
struct Inc {
  T* dst;

  template <typename K>
  void operator()(K ) {
    (*dst)++;
  }
};

template <typename T>
Inc<T> incVisitor(T* dst) {
  return Inc<T>{dst};
}

double twaPrior(Angle<double> twa) {
  return std::pow(0.5*(-cos(twa) + 1), 0.1);
}

Velocity<double> targetSpeedPrior(Angle<double> twa, Velocity<double> tws) {
  return twaPrior(twa)*tws;
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

  RealPerfSurfResults results;

  auto inputData = transduce(
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
      trFilter([&](
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
  results.finalSampleCount = inputData;
  return results;
}

void outputPolars(
    const std::string& prefix,
    const NavDataset& src) {

  auto results = optimizeRealPerfSurf(src);

  std::ofstream info(prefix + "_info.txt");
  info << "Number of samples: " << results.finalSampleCount << std::endl;
}


} /* namespace sail */
