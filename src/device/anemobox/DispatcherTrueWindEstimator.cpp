#include <device/anemobox/DispatcherTrueWindEstimator.h>

#include <device/Arduino/libraries/FixedPoint/FixedPoint.h>
#include <fstream>
#include <device/Arduino/libraries/ChunkFile/ChunkFile.h>

namespace sail {

DispatcherTrueWindEstimator::DispatcherTrueWindEstimator(Dispatcher* dispatcher)
    : _dispatcher(dispatcher),
    _validParameters(false), 
    _validTargetSpeedTable(false),
    _filter(dispatcher, DispatcherFilterParams()) {
}

bool DispatcherTrueWindEstimator::loadCalibration(const std::string& path) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  return loadCalibration(file);
}

bool DispatcherTrueWindEstimator::loadCalibration(std::istream& file) {
  TrueWindEstimator::Parameters<FP16_16> calibration;
  ChunkTarget targets[] = {
    makeChunkTarget(&calibration),
    makeChunkTarget(&_targetSpeedTable)
  };

  ChunkLoader loader(targets, sizeof(targets) / sizeof(targets[0]));

  if (!file.good()) {
    return false;
  }
  while (file.good()) {
    loader.addByte(file.get());
  }

  if (targets[0].success) {
    for (int i = 0; i < TrueWindEstimator::NUM_PARAMS; ++i) {
      _parameters.params[i] = calibration.params[i];
    }
  }
  _validParameters = targets[0].success;

  _validTargetSpeedTable = targets[1].success;
  return _validParameters || _validTargetSpeedTable;
}

void DispatcherTrueWindEstimator::compute() const {
  compute(sourceName());
}

void DispatcherTrueWindEstimator::compute(const std::string &srcName) const {
  Angle<> twdir;
  Angle<> twa;
  Velocity<> tws;

  TimeStamp freshLimit = _dispatcher->currentTime() - Duration<double>::seconds(5);

  if (!_dispatcher->get<GPS_SPEED>()->dispatcher()->hasFreshValue(freshLimit)
      || !_dispatcher->get<AWA>()->dispatcher()->hasFreshValue(freshLimit)
      || !_dispatcher->get<AWS>()->dispatcher()->hasFreshValue(freshLimit)) {
    // we can't compute anything useful without GPS.
    return;
  }

  if (_validParameters) {
    HorizontalMotion<double> wind =
      TrueWindEstimator::computeTrueWind(_parameters.params, _filter);
    twdir = calcTwdir(wind);
    tws = wind.norm();

    _dispatcher->publishValue(TWDIR, srcName, twdir);
    _dispatcher->publishValue(TWS, srcName, tws);

    // Todo: compute TWA with TrueWindEstimator.
    twa = (twdir - _filter.gpsBearing());
    _dispatcher->publishValue(TWA, srcName, twa);
  } else {
    if (!_dispatcher->get<TWA>()->dispatcher()->hasFreshValue(freshLimit)
        || !_dispatcher->get<TWS>()->dispatcher()->hasFreshValue(freshLimit)) {
      return;
    }
    twa = _dispatcher->get<TWA>()->dispatcher()->lastValue();
    tws = _dispatcher->get<TWS>()->dispatcher()->lastValue();
  } 
  
  if (_validTargetSpeedTable) {
    Velocity<> targetVmg = getVmgTarget(_targetSpeedTable, twa, tws);

    // getVmgTarget returns -1 when the value is invalid. In this case,
    // nothing should be published.
    if (targetVmg.knots() >= 0) {
      _dispatcher->publishValue(TARGET_VMG, srcName, targetVmg);
    }
  }

  // TODO: When the TrueWindEstimator will handle current, use water speed
  // instead.
  Velocity<> boatSpeed = _filter.gpsSpeed();

  Velocity<> vmg = cos(twa) * boatSpeed;
  _dispatcher->publishValue(VMG, srcName, vmg);
}

std::string DispatcherTrueWindEstimator::info() const {
  std::string result;

  result += "Calibration parameters: ";
  result += (_validParameters ? "valid" : "invalid");
  result += "\nTarge speed table: ";
  result += (_validTargetSpeedTable ? "valid" : "invalid");
  result += "\n"; 

  if (_validParameters) {
    const double* p = _parameters.params;
    result += 
      stringFormat("  PARAM_AWA_OFFSET: %f\n", p[TrueWindEstimator::PARAM_AWA_OFFSET])
      + stringFormat("  PARAM_UPWIND0: %f\n", p[TrueWindEstimator::PARAM_UPWIND0])
      + stringFormat("  PARAM_DOWNWIND0: %f\n", p[TrueWindEstimator::PARAM_DOWNWIND0])
      + stringFormat("  PARAM_DOWNWIND1: %f\n", p[TrueWindEstimator::PARAM_DOWNWIND1])
      + stringFormat("  PARAM_DOWNWIND2: %f\n", p[TrueWindEstimator::PARAM_DOWNWIND2])
      + stringFormat("  PARAM_DOWNWIND3: %f\n", p[TrueWindEstimator::PARAM_DOWNWIND3]);
  }

  return result;
}

}  // namespace sail
