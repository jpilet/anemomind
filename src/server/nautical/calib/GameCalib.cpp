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
        kv.second.mapper.sampleCount, 2);
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

}

}
}
