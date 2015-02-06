/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Progress.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <cassert>

namespace sail {

Progress::Progress(int totalIterations,
    Duration<double> notificationPeriod) :
    _totalIterations(totalIterations),
    _notificationPeriod(notificationPeriod),
    _counter(0), _offset(TimeStamp::now()) {
  setNextUpdateFromNow();
}


void Progress::setNextUpdateFromNow() {
  _nextUpdate = elapsedTime() + _notificationPeriod;
}

// Once an iteration is complete, call this method. It will
// return true if a notification should be produced.
bool Progress::endOfIteration() {
  _counter++;
  assert(_counter <= _totalIterations);
  if (elapsedTime() >= _nextUpdate) {
    setNextUpdateFromNow();
    return true;
  }
  return false;
}

std::string Progress::iterationMessage() const {
  std::string e = elapsedTime().str();
  std::string r = remainingTime().str();
  return stringFormat("Completed iteration %d/%d (elapsed: %s, remaining: %s)",
          _counter, _totalIterations, e.c_str(), r.c_str());
}

Duration<double> Progress::averageTimePerIteration() const {
  if (_counter == 0) {
    LOG(FATAL) << "Please call endOfIteration() at least once";
  }
  return (1.0/_counter)*elapsedTime();
}



} /* namespace mmm */
