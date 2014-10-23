/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CLEANNUMARRAY_H_
#define CLEANNUMARRAY_H_

#include <server/common/Array.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {

/*
 * Replaces all unusual values in arr
 * (such as inf and nan) with interpolated
 * values.
 */
Arrayd cleanNumArray(Arrayd arr);


/*
 * Takes an array of angles and
 *  (i)  Makes sure they are continuous
 *  (ii) Replaces all unusual values such as inf and nan with interpolated values.
 */
Array<Angle<double> > cleanContinuousAngles(Array<Angle<double> > allAngles);

}

#endif /* CLEANNUMARRAY_H_ */
