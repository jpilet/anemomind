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
  TrueWindEstimator::Parameters<FP16_16> calibration;
  ChunkTarget targets[] = {
    makeChunkTarget(&calibration),
    makeChunkTarget(&_targetSpeedTable)
  };

  ChunkLoader loader(targets, sizeof(targets) / sizeof(targets[0]));

  std::ifstream file(path, std::ios::in | std::ios::binary);
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

  if (!_dispatcher->gpsSpeed()->dispatcher()->hasValue()) {
    // we can't compute anything useful without GPS.
    return;
  }

  if (_validParameters) {
    HorizontalMotion<double> wind =
      TrueWindEstimator::computeTrueWind(_parameters.params, _filter);
    twdir = calcTwdir(wind);
    tws = wind.norm();

    _dispatcher->twdir()->publishValue(sourceName(), twdir);
    _dispatcher->tws()->publishValue(sourceName(), tws);

    // Todo: compute TWA with TrueWindEstimator.
    twa = (twdir - _filter.gpsBearing());
    _dispatcher->twa()->publishValue(sourceName(), twa);
  } else {
    if (!_dispatcher->twa()->dispatcher()->hasValue()
        || !_dispatcher->tws()->dispatcher()->hasValue()) {
      return;
    }
    twa = _dispatcher->twa()->dispatcher()->lastValue();
    tws = _dispatcher->tws()->dispatcher()->lastValue();
  } 
  
  if (_validTargetSpeedTable) {
    Velocity<> targetVmg = getVmgTarget(_targetSpeedTable, twa, tws);
    _dispatcher->targetVmg()->publishValue(sourceName(), targetVmg);
  }

  // TODO: When the TrueWindEstimator will handle current, use water speed
  // instead.
  Velocity<> boatSpeed = _filter.gpsSpeed();

  Velocity<> vmg = cos(twa) * boatSpeed;
  _dispatcher->vmg()->publishValue(sourceName(), vmg);
}

}  // namespace sail
