/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMEOFFSET_H_
#define TIMEOFFSET_H_

#include <ctime>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

/*
 * A simple class for timing things.
 */
class TimeOffset {
 public:
  TimeOffset() {
    _offset = clock();
  }
  Duration<double> elapsed() const {
    clock_t dif = clock() - _offset;
    return Duration<double>::seconds(double(dif)/CLOCKS_PER_SEC);
  }
 private:
  clock_t _offset;
};

}

#endif /* TIMEOFFSET_H_ */
