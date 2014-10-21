/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef CLEANNUMARRAY_H_
#define CLEANNUMARRAY_H_

#include <server/common/Array.h>

namespace sail {

/*
 * Replaces all unusual values in arr
 * (such as inf and nan) with interpolated
 * values.
 */
Arrayd cleanNumArray(Arrayd arr);

}

#endif /* CLEANNUMARRAY_H_ */
