/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARPOINTCONV_H_
#define POLARPOINTCONV_H_

#include <server/nautical/polar/PolarPoint.h>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>

namespace sail {

Array<PolarPoint> navsToPolarPoints(Array<Nav> navs);

}

#endif /* POLARPOINTCONV_H_ */
