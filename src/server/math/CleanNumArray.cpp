/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CleanNumArray.h"
#include <server/math/BandMat.h>

namespace sail {

Arrayd cleanNumArray(Arrayd arr) {
  int count = arr.size();
  BandMatd A(count, count, 2, 2);
}

}
