/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Progress.h>
#include <server/common/logging.h>
#include <cassert>

namespace sail {

Progress::Progress(int totalIterations,
    Duration<double> notificationPeriod) :
    _totalIterations(totalIterations),
    _notificationPeriod(notificationPeriod),
    _counter(0) {
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


} /* namespace mmm */
