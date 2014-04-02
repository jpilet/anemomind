/*
 *  Created on: 2014-03-14
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BandMat.h"

namespace sail {
namespace BandMatInternal {

Arrayd makeNextCoefs(Arrayd coefs) {
  int n = coefs.size();
  Arrayd next(n+1);
  next[n] = 0.0;
  coefs.copyToSafe(next.sliceTo(n));
  for (int i = 0; i < n; i++) {
    next[i+1] -= coefs[i];
  }
  return next;
}

}
}



