/*
 * GameCalib.cpp
 *
 *  Created on: 30 Dec 2016
 *      Author: jonas
 */

#include <server/nautical/calib/GameCalib.h>
#include <server/math/SplineUtils.h>
#include <server/common/string.h>
#include <server/common/indexed.h>
#include <server/math/nonlinear/DataFit.h>
#include <unordered_map>
#include <server/math/SplineUtils.h>

namespace sail {
namespace GameCalib {

void BasicAngleSensor::initialize(double *dst) {
  dst[0] = 0.0;
}

int BasicAngleSensor::parameterCount() const {
  return 1;
}

namespace {
  Angle<adouble> getOffset(adouble *x) {
    return Angle<adouble>::radians(x[0]);
  }
}


Angle<adouble> BasicAngleSensor::correct(
    adouble *parameters,
    Angle<adouble> x) {
  return x - getOffset(parameters);
}

Angle<adouble> BasicAngleSensor::corrupt(
    adouble *parameters,
    Angle<adouble> x) {
  return x + getOffset(parameters);
}

LinearVelocitySensor::LinearVelocitySensor(
    const Array<BasisFunction> &basis) :
      _basis(basis) {}

void LinearVelocitySensor::initialize(double *dst) {
  for (int i = 0; i < _basis.size(); i++) {
    dst[i] = 0.0;
  }
}

int LinearVelocitySensor::parameterCount() const {
  return _basis.size();
}

Velocity<adouble> LinearVelocitySensor::correct(
    adouble *parameters,
    const Velocity<adouble> &x) {
  Velocity<adouble> sum = Velocity<adouble>::metersPerSecond(0.0);
  for (auto kv: indexed(_basis)) {
    sum += parameters[kv.first]*kv.second(x);
  }
  return sum;
}

Velocity<adouble> LinearVelocitySensor::corrupt(
    adouble *parameters,
    const Velocity<adouble> &x) {
  LOG(FATAL) << "This operation is not supported";
  return Velocity<adouble>::metersPerSecond(0.0);
}

struct SplineParam {
  CubicBasis basis;
  TimeMapper mapper;

  CubicBasis::Weights computeWeights(TimeStamp t) const {
    return basis.build(mapper.mapToReal(t));
  }
};

SplineParam allocateSpline(const CalibDataChunk &chunk,
    Duration<double> period) {
  auto span = chunk.span();
  auto dur = span.maxv() - span.minv();
  int n = std::max(1, int(round(dur/period)));
  TimeMapper mapper(span.minv(), (1.0/n)*dur, n);
  return SplineParam{CubicBasis(mapper.sampleCount), mapper};
}

Array<SplineParam> allocateSplines(
    const Array<CalibDataChunk> &chunks,
    Duration<double> period,
    DOM::Node *dst) {
  int n = chunks.size();
  Array<SplineParam> params(n);
  auto ul = DOM::makeSubNode(dst, "ul");
  for (int i = 0; i < n; i++) {
    auto p = allocateSpline(chunks[i], period);
    params[i] = p;
    if (dst) {
      DOM::addSubTextNode(&ul, "li",
          stringFormat("Spline basis of %d samples for "
              "a total duration of %s with a "
              "sampling period of %s",
              p.mapper.sampleCount,
              p.mapper.totalDuration().str().c_str(),
              p.mapper.period.str().c_str()));
    }
  }
  return params;
}

void dispSources(const std::string &title,
    const std::set<std::string> &codes,
    DOM::Node *dst) {
  DOM::addSubTextNode(dst, "h2", title);
  auto ul = DOM::makeSubNode(dst, "ul");
  for (auto x: codes) {
    DOM::addSubTextNode(&ul, "li", x);
  }
}

struct CurrentSetup {
  int totalParamCount = 0;
  Array<SplineParam> currentSplines;
  Array<DataFit::CoordIndexer> currentIndexers;
  std::unordered_map<std::string, DataFit::CoordIndexer>
    magHdgParameterBlocks;
  std::unordered_map<std::string, DataFit::CoordIndexer>
    watSpeedparameterBlocks;
};

std::string toString(const DataFit::CoordIndexer &src) {
  std::stringstream ss;
  ss << "Parameter " << src.from() << " to "
      << src.to() << " with groups of size " << src.dim();
  return ss.str();
}

CurrentSetup makeCurrentSetup(
    const Array<SplineParam> &splineParams,
    const std::set<std::string> &magHdgSources,
    const std::set<std::string> &watSpeedSources,
    const Settings &settings,
    DOM::Node *out) {

  DOM::addSubTextNode(out, "h2", "Current setup");

  CurrentSetup dst;
  dst.currentSplines = splineParams;
  DataFit::CoordIndexer::Factory params;

  Array<DataFit::CoordIndexer> indexers(splineParams.size());
  DOM::addSubTextNode(out, "h3", "Current optimization variables");
  auto olC = DOM::makeSubNode(out, "ol");
  for (auto kv: indexed(splineParams)) {
    auto p = params.make(
        2, kv.second.mapper.sampleCount); // x1.....xN, y1......yN
    indexers[kv.first] = p;
    DOM::addSubTextNode(&olC,
        "li", toString(p));
  }

  int k = params.count();

  for (auto src: magHdgSources) {
    auto p = params.make(1, settings.magHeadingSensor->parameterCount());
    dst.magHdgParameterBlocks.insert({
      src, p});
  }
  for (auto src: watSpeedSources) {
    dst.watSpeedparameterBlocks.insert({
      src,
      params.make(1,
          settings.watSpeedSensor->parameterCount())
    });
  }


  dst.totalParamCount = params.count();
  DOM::addSubTextNode(out, "h3", "Sensor parameters");
  DOM::addSubTextNode(out, "p",
      stringFormat("Sensor parameters from %d "
          "to %d. Number of sensor parameters %d", k,
          dst.totalParamCount, dst.totalParamCount - k));
  DOM::addSubTextNode(out, "h3", "Summary");
  DOM::addSubTextNode(out, "p", stringFormat("Totally %d parameters"
      " to optimize", dst.totalParamCount));

  return dst;
}

int getPlayerIndex(
    const std::set<PlayerType> &players,
    PlayerType type) {
  auto f = players.find(type);
  if (f == players.end()) {
    return -1;
  }
  return std::distance(players.begin(), f);
}

struct OptData {
  Array<CalibDataChunk> chunks;
  CurrentSetup setup;
  std::set<PlayerType> playerSet;
  Settings settings;
  Array<Array<adouble>> parameters;

  Array<adouble> parametersOfType(PlayerType t) const {
    return parameters[getPlayerIndex(playerSet, t)];
  }

  Array<adouble> currentParameters() const {
    return parametersOfType(PlayerType::Current);
  }
};

template <DataCode code>
void forEveryChunkAndChannel(
    const OptData &data,
    const std::unordered_map<std::string,
      DataFit::CoordIndexer> &coordIndexers,
    std::function<void(
        int chunkIndex,
        std::pair<std::string,DataFit::CoordIndexer>,
        Array<TimedValue<typename TypeForCode<code>::type>>)> f) {
  for (auto chunkKV: indexed(data.chunks)) {
    int chunkIndex = chunkKV.first;
    for (auto sensorKV: data.setup.magHdgParameterBlocks) {
      auto field = ChannelFieldAccess<code>::template get(chunkKV.second);
      auto obsf = field->find(sensorKV.first);
      if (obsf != field->end()) {
        f(chunkKV.first, sensorKV, obsf->second);
      }
    }
  }
}

Array<adouble> perDimension(Array<adouble> parameters,
    const DataFit::CoordIndexer &indexer, int dim) {
  auto span = indexer.span(dim);
  return parameters.slice(span.minv(), span.maxv());
}

template <typename T>
Velocity<T> vu() {
  return Velocity<T>::metersPerSecond(1.0);
}

adouble evaluateHeadingFitness(
    const OptData &data) {
  adouble sum = 0.0;
  auto cp = data.currentParameters();
  forEveryChunkAndChannel<MAG_HEADING>(
      data, data.setup.magHdgParameterBlocks,
      [&](int i, const std::pair<std::string,
          DataFit::CoordIndexer> &blk,
          const Array<TimedValue<Angle<double>>> &samples) {
    auto splineParam = data.setup.currentSplines[i];
    auto currentIndexer = data.setup.currentIndexers[i];
    auto Xc = perDimension(cp, currentIndexer, 0);
    auto Yc = perDimension(cp, currentIndexer, 0);
    for (auto obs: samples) {
      auto boatMotion = data.chunks[i].trajectory
          .evaluateHorizontalMotion(obs.time).cast<adouble>();
      auto w = splineParam.computeWeights(obs.time);
      auto currentX = w.evaluateGeneric<adouble>(Xc.getData());
      auto currentY = w.evaluateGeneric<adouble>(Yc.getData());
      HorizontalMotion<adouble> bow(boatMotion
          - HorizontalMotion<adouble>{
        currentX*vu<adouble>(),
        currentY*vu<adouble>()
      });
    }
  });
  return sum;
}

adouble evaluateCurrentPlayer(
    const OptData &data) {
  return evaluateHeadingFitness(data);
}

GameSolver::Function makeCurrentPlayer(
    const Array<CalibDataChunk> &chunks,
    const CurrentSetup &setup,
        const std::set<PlayerType> &playerSet,
        const Settings &settings) {
  return [=](const Array<Array<adouble>> &parameters) {
    return evaluateCurrentPlayer(
        OptData{chunks, setup, playerSet, settings, parameters});
  };
}

void optimize(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings,
    DOM::Node *dst) {
  DOM::addSubTextNode(dst, "h1", "Game based calibration");
  DOM::addSubTextNode(dst, "h2", "Allocate wind splines");
  auto windSplines = allocateSplines(chunks,
      settings.windSamplingPeriod, dst);
  DOM::addSubTextNode(dst, "h2", "Allocate current splines");
  auto currentSplines = allocateSplines(chunks,
      settings.currentSamplingPeriod, dst);

  auto magHdgSources = listSourcesForCode<MAG_HEADING>(chunks);
  auto watSpeedSources = listSourcesForCode<WAT_SPEED>(chunks);

  dispSources("Magnetic heading sources", magHdgSources, dst);
  dispSources("Water speed sources", magHdgSources, dst);

  auto currentSetup = makeCurrentSetup(
      windSplines, magHdgSources, watSpeedSources,
      settings, dst);

  std::set<PlayerType> playerSet{PlayerType::Current};
  Array<GameSolver::Function> objectives(playerSet.size());
  if (0 < playerSet.count(PlayerType::Current)) {
    objectives[getPlayerIndex(playerSet, PlayerType::Current)] =
        makeCurrentPlayer(chunks, currentSetup, playerSet, settings);
  }
}

}
}
