/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/TimeStamp.h>

namespace sail {

/*
 *  Class used to give the user notifications on the progress of an
 *  algorithm that takes long to run.
 *
 *  Usage:
 *
 *  int iters = ... ;
 *  Progress prog(iters);
 *  for (int i = 0; i < iters; i++) {
 *
 *    ... computation ...
 *
 *    if (prog.endOfIteration()) {
 *      std::cout << "Remaining time: " << prog.remainingTime() << std::endl;
 *    }
 *  }
 *
 */
class Progress {
 public:
  Progress(int totalIterations,
      Duration<double> notificationPeriod = Duration<double>::seconds(3.0));

  // Once an iteration is complete, call this method. It will
  // return true if a notification should be produced.
  bool endOfIteration();


  int remainingIterations() const {
    return _totalIterations - _counter;
  }

  Duration<double> elapsedTime() const {
    return TimeStamp::now() - _offset;
  }

  Duration<double> averageTimePerIteration() const {
    return (1.0/_counter)*elapsedTime();
  }

  Duration<double> remainingTime() const {
    return double(remainingIterations())*averageTimePerIteration();
  }

  // How many iterations so far.
  int iterationsSoFar() const {
    return _counter;
  }

  // Total number of iterations
  int totalIterations() const {
    return _totalIterations;
  }
 private:
  TimeStamp _offset;
  Duration<double> _nextUpdate, _notificationPeriod;
  int _counter;
  int _totalIterations;

  void setNextUpdateFromNow();
};

}

#endif /* PROGRESS_H_ */
