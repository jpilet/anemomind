/*
 *  Created on: 2014-03-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  V
 */

#ifndef VARIOUSPLOTS_H_
#define VARIOUSPLOTS_H_

#include <server/nautical/Nav.h>

namespace sail {

void plotPolarAWAAndWatSpeed(Array<Array<Nav> > navs);
void plotPolarAWAAndAWS(Array<Array<Nav> > navs);

} /* namespace mmm */

#endif /* VARIOUSPLOTS_H_ */
