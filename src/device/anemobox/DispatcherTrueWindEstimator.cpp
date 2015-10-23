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
  Angle<> twdir;
  Angle<> twa;
  Velocity<> tws;

  if (!_dispatcher->get<GPS_SPEED>()->dispatcher()->hasValue()
      || !_dispatcher->get<AWA>()->dispatcher()->hasValue()
      || !_dispatcher->get<AWS>()->dispatcher()->hasValue()) {
    // we can't compute anything useful without GPS.
    return;
  }

  if (_validParameters) {
    HorizontalMotion<double> wind =
      TrueWindEstimator::computeTrueWind(_parameters.params, _filter);
    twdir = calcTwdir(wind);
    tws = wind.norm();

    _dispatcher->publishValue(TWDIR, sourceName(), twdir);
    _dispatcher->publishValue(TWS, sourceName(), tws);

    // Todo: compute TWA with TrueWindEstimator.
    twa = (twdir - _filter.gpsBearing());
    _dispatcher->publishValue(TWA, sourceName(), twa);
  } else {
    if (!_dispatcher->get<TWA>()->dispatcher()->hasValue()
        || !_dispatcher->get<TWS>()->dispatcher()->hasValue()) {
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
      _dispatcher->publishValue(TARGET_VMG, sourceName(), targetVmg);
    }
  }

  // TODO: When the TrueWindEstimator will handle current, use water speed
  // instead.
  Velocity<> boatSpeed = _filter.gpsSpeed();

  Velocity<> vmg = cos(twa) * boatSpeed;
  _dispatcher->publishValue(VMG, sourceName(), vmg);
}

}  // namespace sail
