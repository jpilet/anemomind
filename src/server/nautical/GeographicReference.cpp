/*
 *  Created on: 2014-05-21
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeographicReference.h"

namespace sail {

GeographicReference();
GeographicReference(const GeographicPosition<double> &pos);
void map(GeographicPosition<double> src, Length<double> *xyzOut);
void unmap(Length<double> *xyzIn, GeographicPosition<double> *posOut);


} /* namespace sail */
