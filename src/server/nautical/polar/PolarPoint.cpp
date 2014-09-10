/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarPoint.h>

namespace sail {

PolarPoint::PolarPoint(Velocity<double> tws_,
    Angle<double> twa_,
    Velocity<double> boatSpeed_,
    int navIndex_): _twa(twa_),
    _tws(tws_), _boatSpeed(boatSpeed_),
    _navIndex(navIndex_) {}


}




