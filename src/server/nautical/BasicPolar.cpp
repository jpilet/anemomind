/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BasicPolar.h"

namespace sail {

PolarPoint::PolarPoint(Angle<double> twa_,
    Velocity<double> tws_,
    Velocity<double> boatSpeed_,
    int navIndex_): _twa(twa_),
    _tws(tws_), _boatSpeed(boatSpeed_),
    _navIndex(navIndex_) {}

BasicPolar::BasicPolar() {
  // TODO Auto-generated constructor stub

}

BasicPolar::~BasicPolar() {
  // TODO Auto-generated destructor stub
}

} /* namespace mmm */
