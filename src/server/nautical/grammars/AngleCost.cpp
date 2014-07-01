/*
 *  Created on: 2014-06-30
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "AngleCost.h"

namespace sail {


void AngleCost::add(int stateIndex, Angle<double> angle) {
  _anglePerState[stateIndex] = angle;
}

double AngleCost::calcCost(int stateIndex, Angle<double> angle) const {
  if (_anglePerState.find(stateIndex) == _anglePerState.end()) {
    return 0;
  }

  const Angle<double> &b = _anglePerState.at(stateIndex);
  return std::abs((angle.directionDifference(b).radians())/M_PI);
}


} /* namespace mmm */
