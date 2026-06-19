#ifndef ANEMOBOX_DISPATCHER_TRUE_WIND_ESTIMATOR_H
#define ANEMOBOX_DISPATCHER_TRUE_WIND_ESTIMATOR_H

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/DispatcherFilter.h>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>

#include <string>

namespace sail {

class DispatcherTrueWindEstimator {
 public:
  DispatcherTrueWindEstimator(Dispatcher *dispatcher);

  bool loadCalibration(const std::string& path);
  bool loadCalibration(std::istream& file);

  // Compute and publish using the default source name
  void compute() const;

  // Compute and publish using a specific source name
  void compute(const std::string &srcName) const;

  static const char* sourceName() { return "Anemomind estimator"; }

  std::string info() const;

 private:
  Dispatcher* _dispatcher;
  TrueWindEstimator::Parameters<double> _parameters;

  // True as soon as _parameters can be used to compute true wind. This is the
  // case right from construction, because we start with default parameters.
  bool _validParameters;

  // True only once a calibration has actually been loaded from a file. When
  // false, _parameters holds the default (uncalibrated) values.
  bool _calibrated;

  TargetSpeedTable _targetSpeedTable;
  bool _validTargetSpeedTable;

  DispatcherFilter _filter;
};

}  // namespace sail

#endif // ANEMOBOX_DISPATCHER_TRUE_WIND_ESTIMATOR_H
