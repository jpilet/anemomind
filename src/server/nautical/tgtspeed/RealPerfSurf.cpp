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
  return twaPrior(twa)*tws + 1.0e-9_kn;
}

std::array<Velocity<double>, 2> toCartesian(
    Angle<double> twa, Velocity<double> tws) {
  return {sin(twa)*tws, cos(twa)*tws};
}

Eigen::MatrixXd makeFirstOrderReg(
    int vertexCount,
    const Array<std::pair<int, int>>& edges) {
  int edgeCount = edges.size();
  Eigen::MatrixXd dst = Eigen::MatrixXd::Zero(edgeCount, vertexCount);
  for (int i = 0; i < edges.size(); i++) {
    auto e = edges[i];
    dst(i, e.first) = 1.0;
    dst(i, e.second) = -1.0;
  }
  return dst;
}

//return makeDataPoint(
//    twa, tws, boatSpeed, hexMesh, speedUnit);

struct DataPoint {
  double normalizedBoatSpeed = 0.0;
  std::array<WeightedIndex, 3> vertexWeights;
};

Optional<DataPoint> makeDataPoint(
    TimedValue<Angle<double>> twa,
    TimedValue<Velocity<double>> tws,
    TimedValue<Velocity<double>> boatSpeed,
    HexMesh mesh, Velocity<double> meshGridUnit) {
  auto local = toCartesian(twa.value, tws.value);
  auto weights = mesh.represent(
      Eigen::Vector2d(local[0]/meshGridUnit,
      local[1]/meshGridUnit));
  if (weights.undefined()) {
    return {};
  }

  DataPoint dst;
  dst.vertexWeights = weights.get();
  dst.normalizedBoatSpeed = boatSpeed.value/targetSpeedPrior(
      twa.value, tws.value);
  return dst;
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

  HexMesh mesh(25, 25.0);

  auto meshGridUnit = 1.0_mps;

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
        return makeDataPoint(
            twa, tws, boatSpeed,
            mesh, meshGridUnit);
      })
      |
      trCat(),
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
