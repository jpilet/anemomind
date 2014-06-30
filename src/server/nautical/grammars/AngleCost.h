/*
 *  Created on: 2014-06-30
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ANGLECOST_H_
#define ANGLECOST_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <map>

namespace sail {

class AngleCost {
 public:
  // Adds a new angle to the set of angles
  void add(int stateIndex, Angle<double> angle);

  // Computes a part of the cost of being in a state with index 'stateIndex'
  // with an angle of 'angle'
  double calcCost(int stateIndex, Angle<double> angle) const;
 private:
  std::map<int, Angle<double> > _anglePerState;
};

}

#endif
