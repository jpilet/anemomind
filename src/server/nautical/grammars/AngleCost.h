/*
 *  Created on: 2014-06-30
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ANGLECOST_H_
#define ANGLECOST_H_

#include <server/common/PhysicalQuantity.h>

namespace sail {

class AngleCost {
 public:
  AngleCost();
  virtual ~AngleCost();
 private:
  std::map<int, Angle<double> > _anglePerState;
};

}

#endif
