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

 private:
  Dispatcher* _dispatcher;
  TrueWindEstimator::Parameters<double> _parameters;
  bool _validParameters;

  TargetSpeedTable _targetSpeedTable;
  bool _validTargetSpeedTable;

  DispatcherFilter _filter;
};

}  // namespace sail

#endif // ANEMOBOX_DISPATCHER_TRUE_WIND_ESTIMATOR_H
